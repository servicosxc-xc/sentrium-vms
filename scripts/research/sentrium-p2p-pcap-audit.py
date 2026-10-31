#!/usr/bin/env python3
"""Read-only, dependency-free PCAPNG metadata audit for SENTRIUM F005 research.

Privacy contract: never write IP/MAC addresses, DNS response contents, traffic
payloads, device serials, tokens or client credentials. Public hostnames are
printed ONLY when matching a small reviewed exact-name allowlist. This tool
observes frame timing and envelopes; it does not decode app authentication.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import ipaddress
import json
from pathlib import Path
import struct

APPROVED_DNS = frozenset({
    "intelbrasp2p.com.br",
    "api-cronos.intelbras.com.br",
    "api.simnext.isec.intelbras.com.br",
    "cronos-simnext.intelbras.com.br",
    "logs.isec.intelbras.com.br",
})
MAX_INPUT = 256 * 1024 * 1024
MAX_BLOCK = 16 * 1024 * 1024


def pcapng_frames(path: Path):
    if not path.is_file() or path.stat().st_size > MAX_INPUT:
        raise ValueError("Input absent or exceeds 256 MiB limit")
    interfaces: list[tuple[int, float]] = []
    with path.open("rb") as stream:
        while True:
            head = stream.read(8)
            if not head:
                return
            if len(head) != 8:
                raise ValueError("Truncated PCAPNG block header")
            kind, size = struct.unpack("<II", head)
            if size < 12 or size > MAX_BLOCK or size % 4:
                raise ValueError("Invalid PCAPNG block size")
            block = stream.read(size - 8)
            if len(block) != size - 8 or struct.unpack_from("<I", block, len(block)-4)[0] != size:
                raise ValueError("Truncated PCAPNG block or mismatched trailer")
            body = memoryview(block)[:-4]
            if kind == 0x0A0D0D0A:
                if len(body) < 16 or struct.unpack_from("<I", body, 0)[0] != 0x1A2B3C4D:
                    raise ValueError("Only little-endian PCAPNG is supported")
                interfaces.clear()
            elif kind == 1:
                if len(body) < 8:
                    raise ValueError("Truncated interface block")
                linktype = struct.unpack_from("<H", body)[0]
                resolution = 1e-6
                offset = 8
                while offset + 4 <= len(body):
                    option, length = struct.unpack_from("<HH", body, offset)
                    offset += 4
                    if offset + length > len(body):
                        raise ValueError("Truncated interface option")
                    if option == 9 and length == 1:
                        exponent = body[offset] & 127
                        resolution = (2 if body[offset] & 128 else 10) ** (-exponent)
                    offset += (length + 3) & ~3
                    if option == 0:
                        break
                interfaces.append((linktype, resolution))
            elif kind == 6:
                if len(body) < 20:
                    raise ValueError("Truncated enhanced packet block")
                iface, hi, lo, captured, original = struct.unpack_from("<IIIII", body)
                if iface >= len(interfaces) or captured > len(body) - 20:
                    raise ValueError("Invalid packet interface/length")
                linktype, resolution = interfaces[iface]
                timestamp = ((hi << 32) | lo) * resolution
                yield timestamp, linktype, body[20:20+captured], original


def ip_packet(frame: memoryview, linktype: int):
    if linktype != 1 or len(frame) < 14:
        return None
    ethertype = struct.unpack_from("!H", frame, 12)[0]
    offset = 14
    while ethertype in (0x8100, 0x88a8):
        if len(frame) < offset + 4:
            return None
        ethertype = struct.unpack_from("!H", frame, offset + 2)[0]
        offset += 4
    if ethertype == 0x0800:
        if len(frame) < offset + 20 or (frame[offset] >> 4) != 4:
            return None
        hsize = (frame[offset] & 15)*4
        plen = struct.unpack_from("!H", frame, offset+2)[0]
        frag = struct.unpack_from("!H", frame, offset+6)[0]
        if hsize < 20 or frag & 0x1FFF or plen < hsize:
            return None
        proto = int(frame[offset+9])
        src = bytes(frame[offset+12:offset+16]); dst = bytes(frame[offset+16:offset+20])
        start = offset+hsize; end = min(len(frame), offset+plen)
        family = 4
    elif ethertype == 0x86DD:
        if len(frame) < offset + 40 or frame[offset] >> 4 != 6:
            return None
        # No IPv6 extension-header parsing: classify only immediate TCP/UDP.
        proto = int(frame[offset+6])
        src = bytes(frame[offset+8:offset+24]); dst = bytes(frame[offset+24:offset+40])
        start = offset+40; end = min(len(frame), start + struct.unpack_from("!H",frame,offset+4)[0]);family=6
    else:
        return None
    if proto not in (6,17) or end < start + 8:
        return (proto, family, src, dst, 0, 0, b"", 0)
    sport, dport = struct.unpack_from("!HH", frame, start)
    if proto == 17:
        payload = bytes(frame[start+8:end]); flags = 0
    else:
        if end < start + 20: return None
        header = (frame[start+12] >> 4)*4
        if header < 20 or end < start+header: return None
        payload = bytes(frame[start+header:end]); flags = int(frame[start+13])
    return proto, family, src, dst, sport, dport, payload, flags


def dns_question(raw: bytes):
    if len(raw) < 12 or struct.unpack_from("!H",raw,2)[0]&0x8000 or not struct.unpack_from("!H",raw,4)[0]:
        return None
    pos=12; labels=[]
    while pos < len(raw):
        n=raw[pos];pos+=1
        if n == 0:break
        if n & 0xC0 or n > 63 or pos+n > len(raw):return None
        try:labels.append(raw[pos:pos+n].decode("ascii").lower())
        except UnicodeDecodeError:return None
        pos+=n
    else:
        return None
    return ".".join(labels)


def _dns_name(raw: bytes, offset: int, depth=0):
    """Bounded DNS name parser; follows compression pointers without exporting names."""
    if depth > 10:
        return None, offset
    labels=[]
    while offset < len(raw):
        length=raw[offset];offset+=1
        if length == 0:
            return ".".join(labels),offset
        if length & 0xC0 == 0xC0:
            if offset >= len(raw):return None,offset
            target=((length&0x3F)<<8)|raw[offset];offset+=1
            result,_=_dns_name(raw,target,depth+1)
            if result is None:return None,offset
            labels.append(result)
            return ".".join(labels),offset
        if length & 0xC0 or length > 63 or offset+length > len(raw):
            return None,offset
        try:labels.append(raw[offset:offset+length].decode("ascii").lower())
        except UnicodeDecodeError:return None,offset
        offset+=length
    return None,offset


def reviewed_dns_reply_addresses(raw: bytes):
    """Return only addresses of ANSWER RRs for exact approved vendor queries.

    Addresses stay in process memory for transient correlation and are not
    serialized. No generic DNS contents, CNAMEs or hostname labels are logged.
    """
    if len(raw)<12:return None
    _,flags,qd,an,_,_=struct.unpack_from("!HHHHHH",raw)
    if not flags & 0x8000 or qd<1 or qd>8 or an>64:return None
    offset=12;queried=[]
    for _ in range(qd):
        hostname,offset=_dns_name(raw,offset)
        if hostname is None or offset+4>len(raw):return None
        offset+=4
        queried.append(hostname)
    approved=[h for h in queried if h in APPROVED_DNS]
    if not approved:return None
    answers=[]
    for _ in range(an):
        _,offset=_dns_name(raw,offset)
        if offset+10>len(raw):break
        rtype,rclass,_,rdlen=struct.unpack_from("!HHIH",raw,offset)
        offset+=10
        if offset+rdlen>len(raw):break
        if rclass==1 and ((rtype==1 and rdlen==4) or (rtype==28 and rdlen==16)):
            answers.append(raw[offset:offset+rdlen])
        offset+=rdlen
    return approved,answers


def public_addr(addr: bytes):
    return ipaddress.ip_address(addr).is_global


def known_kind(payload: bytes):
    if len(payload)>=4 and payload[:3] == b"\x12\x01\x10" and payload[3] in range(7):
        return f"12 01 10 {payload[3]:02X}"
    if len(payload)>=4 and payload[:2] == b"\x12\x20" and payload[3] == 7 and payload[2] in (0xD0,0xD1,0xD2):
        return f"12 20 {payload[2]:02X} 07"
    return "other"


def audit(path: Path):
    frames=0; ip_counts=Counter(); first=None; last=None; tls_syn=[]; vendor_dns=defaultdict(list)
    dns_address_labels=defaultdict(set); type02_total=0; type02_valid=0
    udp_flows={}
    for ts, linktype, frame, original_len in pcapng_frames(path):
        if first is None:first=ts
        last=ts;frames+=1
        parsed=ip_packet(frame,linktype)
        if parsed is None:
            continue
        proto, family, src, dst, sport, dport, payload, flags = parsed
        ip_counts[f"ipv{family}_"+({6:"tcp",17:"udp"}.get(proto,"other"))]+=1
        relative=ts-first
        if proto==17 and dport==53:
            q=dns_question(payload)
            if q in APPROVED_DNS:
                vendor_dns[q].append(round(relative,3))
        if proto==17 and sport==53:
            answer=reviewed_dns_reply_addresses(payload)
            if answer:
                hosts,addrs=answer
                for addr in addrs:
                    dns_address_labels[addr].update(hosts)
        if proto==6 and (flags&2) and not (flags&16) and dport==443 and public_addr(dst):
            tls_syn.append((round(relative,3), dst))
        if proto!=17:continue
        endpoints=sorted(((src,sport),(dst,dport)))
        key=(family,tuple(endpoints))
        is_forward=(src,sport)==endpoints[0]
        flow=udp_flows.setdefault(key,{"count":0,"bytes":0,"start":relative,"end":relative,"by_second":Counter(),
                                     "kind_count":Counter(),"events":[],"all_events":[],"direction_counts":Counter()})
        flow["count"]+=1;flow["bytes"]+=original_len;flow["end"]=relative
        flow["by_second"][int(relative)]+=original_len
        tag=known_kind(payload);flow["kind_count"][tag]+=1
        if tag == "12 01 10 02":
            type02_total += 1
            if len(payload)>=16 and struct.unpack_from("!H",payload,8)[0] == len(payload)-16:
                type02_valid += 1
        flow["direction_counts"]["A_to_B" if is_forward else "B_to_A"]+=1
        evt=(relative,"A_to_B" if is_forward else "B_to_A",tag,len(payload),src,dst,sport,dport)
        if len(flow["events"])<45:flow["events"].append(evt)
        # Only retain relevant first ~80s for comparison; no payload.
        if len(flow["all_events"])<15000:flow["all_events"].append(evt)
    if first is None:raise ValueError("No Enhanced Packet Blocks in PCAPNG")
    if not udp_flows: raise ValueError("No UDP conversations in capture")
    key,main=max(udp_flows.items(),key=lambda x:x[1]["bytes"])
    a,b=key[1];global_a=public_addr(a[0]);global_b=public_addr(b[0])
    if global_a!=global_b:
        remote=(a if global_a else b);local=(b if global_a else a)
    else:
        remote=b;local=a
    direction=lambda evt: "remote_to_local" if (evt[4],evt[6])==remote else "local_to_remote"
    bysec=main["by_second"]
    high=[s for s in sorted(bysec) if bysec[s] >= 80000 and bysec[s+1]>=80000]
    burst=high[0] if high else None
    large=[evt[0] for evt in main["all_events"] if evt[3]>=900 and direction(evt)=="remote_to_local"]
    # Same WAN/remote endpoint other UDP ports, useful for initial probe identification.
    neighbors=[]
    for other_key, other in udp_flows.items():
        if other_key==key:continue
        if remote[0] not in (other_key[1][0][0],other_key[1][1][0]):continue
        remote_sock=next((s for s in other_key[1] if s[0]==remote[0]),None)
        if remote_sock is None:continue
        neighbors.append({"remotePort":remote_sock[1],"firstSecond":round(other["start"],3),
                          "packets":other["count"],"firstKind":other["events"][0][2]})
    neighbors.sort(key=lambda x:x["firstSecond"])
    early=[]
    for ev in main["events"][:25]:
        early.append({"second":round(ev[0],3),"direction":direction(ev),"prefix":ev[2],"udpBytes":ev[3]})
    top_kinds=dict(main["kind_count"].most_common())
    tls_vendor=defaultdict(list)
    for second,dst in tls_syn:
        for hostname in dns_address_labels.get(dst,()):
            tls_vendor[hostname].append(second)
    return {"schema":"sentrium.f005.pcap.metadata.v1","privacy":"No addresses, payloads, serials, tokens or customer identifiers",
            "frames":frames,"durationSeconds":round(last-first,3),"networkProtocols":dict(ip_counts),
            "observedType02LengthField":{"checked":type02_total,"matched16ByteHeader":type02_valid},
            "vendorDnsQueries":[{"hostname":h,"firstSecond":min(t),"count":len(t)} for h,t in sorted(vendor_dns.items(),key=lambda x:min(x[1]))],
            "tcp443SynSeconds":[sec for sec,_ in tls_syn[:40]],
            "tcp443SynMatchedToVendorDns":[{"hostname":name,"firstSecond":min(times),"synCount":len(times)}
                for name,times in sorted(tls_vendor.items(),key=lambda pair:min(pair[1]))],
            "tcp443UnmatchedSynCount":sum(1 for _,dst in tls_syn if dst not in dns_address_labels),
            "dominantUdp":{"localPort":local[1],"remotePort":remote[1],
              "firstSecond":round(main["start"],3),"lastSecond":round(main["end"],3),
              "packets":main["count"],"frameBytes":main["bytes"],"kindCounts":top_kinds,
              "firstPossibleSustainedMediaSecond":burst,"firstReceivedLargeDatagramSecond":round(min(large),3) if large else None,
              "firstPackets":early,
              "neighborRemoteUdpPorts":neighbors[:12],
              "oneSecondFrameBytes":[{"second":sec,"bytes":bysec[sec]} for sec in sorted(bysec)]},
            "limitations":["This is passive correlation, not a specification of Cloud authentication or Direct/Relay semantics.",
                           "No packet contents, endpoint IPs or credentials are retained in output.",
                           "Known manufacturer DNS queries do not prove which server authenticated the device.",
                           "UDP port pairs are session-specific and may change."]}


def main():
    parser=argparse.ArgumentParser(description="Sanitized offline F005 PCAPNG timeline; never emits IPs/payloads")
    parser.add_argument("pcap",type=Path,help="local PCAPNG file (remains unchanged)")
    parser.add_argument("--output",type=Path,help="optional destination for sanitized JSON")
    args=parser.parse_args()
    result=audit(args.pcap)
    data=json.dumps(result,ensure_ascii=False,indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(data+"\n",encoding="utf-8")
        print("SANITIZED report created at",args.output)
    else: print(data)

if __name__=="__main__":main()

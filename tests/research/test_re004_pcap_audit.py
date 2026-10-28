"""Synthetic, non-sensitive unit tests for the RE-004 passive PCAPNG analyzer."""
import importlib.util
import json
from pathlib import Path
import socket
import struct
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[2] / "scripts" / "research" / "sentrium-p2p-pcap-audit.py"
spec = importlib.util.spec_from_file_location("sentrium_re004_pcap", SCRIPT)
audit_lib = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit_lib)


def block(block_type, payload):
    padding = b"\0" * ((4-len(payload)%4)%4)
    length = 12+len(payload)+len(padding)
    return struct.pack("<II",block_type,length)+payload+padding+struct.pack("<I",length)


def packet(src, dst, sport, dport, payload):
    ip_src = socket.inet_aton(src)
    ip_dst = socket.inet_aton(dst)
    ip_len = 20+8+len(payload)
    ipv4 = struct.pack("!BBHHHBBH4s4s",0x45,0,ip_len,0,0,64,17,0,ip_src,ip_dst)
    udp = struct.pack("!HHHH",sport,dport,8+len(payload),0)
    return b"\x02"*6 + b"\x01"*6 + b"\x08\x00" + ipv4 + udp + payload


def emit_pcapng(records):
    # 1 us timestamp resolution. A malicious fixture isn't allowed to run code.
    section = block(0x0A0D0D0A,struct.pack("<IHHq",0x1A2B3C4D,1,0,-1))
    interface = block(1,struct.pack("<HHI",1,0,65535))
    parts=[section,interface]
    for seconds,raw in records:
        ticks=round(seconds*1e6)
        epb=struct.pack("<IIIII",0,ticks>>32,ticks&0xffffffff,len(raw),len(raw))+raw
        parts.append(block(6,epb))
    return b"".join(parts)


def type02(payload_length,valid=True):
    data_len=payload_length-16
    declared=data_len if valid else data_len+1
    return b"\x12\x01\x10\x02" + struct.pack("!I",13)+struct.pack("!H",declared) + b"\0"*6 + b"X"*data_len


class PassiveAuditTests(unittest.TestCase):
    def write(self, raw):
        tmp=tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        file=Path(tmp.name)/"synthetic.pcapng"
        file.write_bytes(raw)
        return file

    def test_synthetic_udp_and_privacy(self):
        local="192.168.10.2";remote="8.8.8.8"
        records=[
            (1,packet(local,remote,64220,1639,b"\x12\x20\xd0\x07"+b"\0"*20)),
            (2,packet(remote,local,1639,64220,type02(1400))),
            (3,packet(remote,local,1639,64220,type02(1400,False))),
            (4,packet(local,remote,64220,1639,b"\x12\x01\x10\x03"+b"\0"*12)),
        ]
        output=audit_lib.audit(self.write(emit_pcapng(records)))
        flow=output["dominantUdp"]
        self.assertEqual(output["frames"],4)
        self.assertEqual(flow["remotePort"],1639)
        self.assertEqual(flow["localPort"],64220)
        self.assertEqual(flow["packets"],4)
        self.assertEqual(output["observedType02LengthField"],{"checked":2,"matched16ByteHeader":1})
        report=json.dumps(output)
        self.assertNotIn(local,report)
        self.assertNotIn(remote,report)
        self.assertNotIn("XXXX",report)
        self.assertEqual(flow["firstPackets"][1]["direction"],"remote_to_local")
        self.assertEqual(flow["firstPackets"][0]["prefix"],"12 20 D0 07")

    def test_dns_exact_allowlist(self):
        def dns(name):
            labels=b"".join(bytes([len(x)])+x.encode('ascii') for x in name.split('.'))+b"\0"
            return struct.pack('!HHHHHH',1,0x0100,1,0,0,0)+labels+struct.pack('!HH',1,1)
        records=[
            (1,packet('192.168.10.2','8.8.8.8',54000,53,dns('intelbrasp2p.com.br'))),
            (1.5,packet('192.168.10.2','8.8.8.8',54001,53,dns('serial-SECRET.client.example'))),
            (2,packet('8.8.8.8','192.168.10.2',1639,64220,type02(200)))
        ]
        data=audit_lib.audit(self.write(emit_pcapng(records)))
        self.assertEqual(data['vendorDnsQueries'][0]['hostname'],'intelbrasp2p.com.br')
        self.assertNotIn('serial-SECRET',json.dumps(data))

    def test_dns_response_to_https_syn_correlates_without_ip_output(self):
        hostname='api-cronos.intelbras.com.br'
        labels=b''.join(bytes([len(x)])+x.encode('ascii') for x in hostname.split('.'))+b'\0'
        q=labels+struct.pack('!HH',1,1)
        query=struct.pack('!HHHHHH',11,0x0100,1,0,0,0)+q
        answer=struct.pack('!HHHHHH',11,0x8180,1,1,0,0)+q+b'\xc0\x0c'+struct.pack('!HHIH',1,1,60,4)+socket.inet_aton('8.8.4.4')
        def tcp_syn():
            ip=struct.pack('!BBHHHBBH4s4s',0x45,0,40,0,0,64,6,0,socket.inet_aton('192.168.10.2'),socket.inet_aton('8.8.4.4'))
            tcp=struct.pack('!HHIIHHHH',50000,443,0,0,(5<<12)|0x002,65535,0,0)
            return b'\x02'*6+b'\x01'*6+b'\x08\x00'+ip+tcp
        records=[
            (1,packet('192.168.10.2','8.8.8.8',51000,53,query)),
            (2,packet('8.8.8.8','192.168.10.2',53,51000,answer)),
            (3,tcp_syn()),
            (4,packet('8.8.4.4','192.168.10.2',1639,64220,type02(200))),
        ]
        result=audit_lib.audit(self.write(emit_pcapng(records)))
        matched=result['tcp443SynMatchedToVendorDns']
        self.assertEqual(matched,[{'hostname':hostname,'firstSecond':2.0,'synCount':1}])
        self.assertEqual(result['tcp443UnmatchedSynCount'],0)
        self.assertNotIn('8.8.4.4',json.dumps(result))

    def test_rejects_malformed_block(self):
        raw=bytearray(emit_pcapng([(1,packet('192.168.10.2','8.8.8.8',54000,1639,b'x'))]))
        raw[-4:]=struct.pack('<I',9999)
        with self.assertRaises(ValueError):
            audit_lib.audit(self.write(raw))


if __name__=='__main__':
    unittest.main()

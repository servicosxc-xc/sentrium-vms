# SENTRIUM-CPP-Foundation-004 — FFmpeg Compatibility Result

**Registro:** F004-MEDIA-FFMPEG-001  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-004 — Media Lab  
**Data:** 25/10/2026  
**Estado:** **PASS — DHAV + FFmpeg software + D3D11VA supported**

## 1. Amostra

Arquivo:

`principal-ch01.raw`

Tamanho:

`2,159,842 bytes`

Origem:

NetSDK LAN / Principal / canal real, capturado pelo Media Lab sem forçar formato.

## 2. Toolchain FFmpeg

- FFmpeg: `9.0.2-essentials_build-www.gyan.dev`
- ffprobe: `9.0.2-essentials_build-www.gyan.dev`
- ffprobe SHA-256: `F0D36ECBBDD3BCFAC3EFA078C96C7271C2E68B3810595552AC3B7F17E9A65C52`
- ffmpeg SHA-256: `3256173F3F8BFFD7DF12227C68ADF68025EDB1832273A9530688A7BB1ED8EDEC`

Package ZIP previously validated against:

`60F467265B1E312373DBCD92200C2618A74850F98D3D078E94296BB3FA2047BA`

## 3. Demux

`ffprobe` exited with code 0 without forcing input format.

Detected:

- format: `dhav`
- long name: `Video DAV`
- codec: `h264`
- codec long name: `H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10`
- resolution: `1280×720`
- pixel format: `yuv420p`

Decision:

`DemuxDecision = SUPPORTED`

This proves the NetSDK callback sample is directly consumable by FFmpeg's DHAV demuxer for this tested stream. No custom parser or Annex-B extraction was required.

## 4. Software decode

FFmpeg software decode completed with exit code 0.

Decision:

`SoftwareDecodeDecision = SUPPORTED`

## 5. D3D11VA

FFmpeg D3D11VA decode completed with exit code 0 after software decode had already passed.

Decision:

`D3D11VADecision = SUPPORTED`

This result proves functional compatibility for the tested sample. It does **not** yet establish a performance advantage, zero-copy path, or production selection.

## 6. F004 acceptance impact

Evidence supports:

- F004-AC-09: PASS for this sample — RAW/DHAV identified;
- F004-AC-10: PASS — FFmpeg software = SUPPORTED;
- F004-AC-11: PASS — D3D11VA = SUPPORTED.

Still pending before media-pipeline recommendation:

- controlled performance comparison;
- process CPU/memory observations;
- render-path implications;
- final recommendation versus PlaySDK oracle.

## 7. Conclusion

For the tested Intelbras/Dahua Principal stream:

`NetSDK RAW/DHAV → FFmpeg DHAV demux → H.264 decode`

is directly viable.

Both CPU software decode and D3D11VA hardware decode are functionally viable candidates for SENTRIUM. Backend selection remains undecided until measured.

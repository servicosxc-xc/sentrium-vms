# SENTRIUM-CPP-Foundation-004 — FFmpeg Probe Baseline

**Estado:** AUTHORIZED / IN IMPLEMENTATION  
**Data:** 25/10/2026

## Baseline externo fixado

Para os probes da F004:

- FFmpeg: **9.0.2**
- arquitetura: **Windows x86_64 / AMD64**
- distribuição de referência: **Gyan release essentials ZIP**
- package SHA-256:  
  `60f467265b1e312373dbcd92200c2618a74850f98d3d078e94296bb3fa2047ba`

O pacote é externo ao repositório e deve ser apontado por:

`SENTRIUM_FFMPEG_ROOT`

O probe rejeita outra versão.

## Por que essentials

O objetivo da F004 é somente:

- ffprobe;
- software decode;
- D3D11VA.

O build essentials contém ffmpeg/ffprobe e inclui suporte de hardware Windows necessário, incluindo D3D11VA. Não há razão para incorporar bibliotecas extras do full build nesta etapa.

## Script

`scripts/windows/sentrium-ffmpeg-probe.ps1`

Uso:

```powershell
$env:SENTRIUM_FFMPEG_ROOT = "C:\Temp\FFmpeg\ffmpeg-9.0.2-essentials_build"

& ".\scripts\windows\sentrium-ffmpeg-probe.ps1" `
    -InputPath ".\out\sentrium-debug\principal-ch01.raw"
```

O script:

1. valida `ffmpeg.exe` e `ffprobe.exe` AMD64;
2. exige FFmpeg/ffprobe 9.0.2;
3. executa ffprobe sem forçar container/codec;
4. só executa software decode se um stream de vídeo for identificado;
5. só executa D3D11VA se software decode passar;
6. grava evidências em uma pasta `ffmpeg-probe` ao lado da captura;
7. não extrai H.264 por heurística;
8. não contém host, usuário ou senha.

## Decisões permitidas

### Demux
- SUPPORTED
- UNSUPPORTED

### Software decode
- SUPPORTED
- UNSUPPORTED
- NOT RUN

### D3D11VA
- SUPPORTED
- UNSUPPORTED
- NOT APPLICABLE

Resultado negativo é evidência válida e não autoriza criação de parser DHAV próprio nesta Foundation.

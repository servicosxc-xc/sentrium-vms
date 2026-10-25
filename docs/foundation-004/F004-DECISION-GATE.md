# F004 MEDIA LAB — DECISION GATE 0.1

**Estado:** AUTHORIZED FOR EXECUTION  
**Foundation:** SENTRIUM-CPP-Foundation-004 — Media Lab

## Objetivo do gate

Autorizar um laboratório controlado de mídia para escolher a pipeline da futura Foundation-005 Live View.

## Se autorizado

A execução poderá:

- adicionar `CLIENT_RealPlayEx`, callback RAW/DHAV e `CLIENT_StopRealPlay`;
- integrar o subset necessário do PlaySDK;
- criar `sentrium-media-lab`;
- capturar amostras sanitizadas de stream;
- integrar FFmpeg externo para probes;
- testar software decode e D3D11VA quando tecnicamente aplicáveis;
- medir 1/4/16 streams onde possível;
- produzir CSV/JSON de benchmark;
- executar testes no NVR Intelbras LAN já validado.

## Não autorizado por este gate

- Cloud/P2P;
- Live View de produção;
- mosaico definitivo;
- gravação/playback;
- IA/LPR;
- redistribuição automática de SDK/FFmpeg;
- promoção automática de ADR-005;
- merge na `main` antes do Human Review.

## Estado atual

**FOUNDATION-004 — AUTHORIZED FOR EXECUTION**


## Execution Direction Update — 25/10/2026

Because the current LAN recorder does not provide enough active cameras for representative scale testing, the authorized execution order is refined:

1. finish F004 media compatibility/decision work with the available real camera streams;
2. do not force 4/16-camera performance conclusions from empty or inactive channels;
3. prepare the next Foundation around real device access through LAN, Cloud/P2P and automatic registration, surfaced through the SENTRIUM client;
4. resume representative 16/32-channel scaling only after complete recorders are reachable.

Cloud/P2P and automatic registration are **not authorized inside F004** by this note. They belong to the next Foundation and require their own scope/gate.

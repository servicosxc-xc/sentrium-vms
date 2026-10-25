# F004 MEDIA LAB — DECISION GATE 0.1

**Estado:** PROPOSED  
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

**FOUNDATION-004 — NOT YET AUTHORIZED FOR IMPLEMENTATION**

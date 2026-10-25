# SENTRIUM-CPP-Foundation-004 — 4x EXTRA MULTI-STREAM VALIDATION

**Registro:** F004-MEDIA-REAL-003  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-004 — Media Lab  
**Data:** 25/10/2026  
**Estado:** **PASS — 4× Extra / PlaySDK / NetSDK-LAN**

## 1. Cenário

Um único processo `sentrium-media-lab`, um único login NetSDK no NVR e quatro canais simultâneos:

- canais: 0, 1, 2, 3;
- stream: Extra;
- duração nominal: 10 s;
- backend: PlaySDK headless;
- transporte: NetSDK LAN.

## 2. Resultado agregado

- Result: PASS;
- ChannelCount: 4;
- wall time: ~10.94 s;
- packets: 147;
- bytes: 506,982;
- throughput agregado observado: ~0.371 Mb/s;
- decoded callbacks: 121;
- decoded FPS agregado por wall time: ~11.06;
- queue drops: 0;
- max queue HWM: 3;
- input retries: 0;
- input failures: 0.

## 3. Resultado por canal

### Canal 0
- PASS;
- 704×480;
- 3 fps declarados;
- ~3.10 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

### Canal 1
- PASS;
- 704×480;
- 3 fps declarados;
- ~3.00 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

### Canal 2
- PASS;
- 704×480;
- 3 fps declarados;
- ~3.00 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

### Canal 3
- PASS;
- 704×480;
- 3 fps declarados;
- ~3.00 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

## 4. Telemetria de captura — defeito encontrado

O operador removeu `SENTRIUM_MEDIA_CAPTURE` antes deste cenário, porém o JSON reportou `CaptureWritten=true` com bytes não nulos.

Causa identificada no lab: uso do estado lógico de um `std::ofstream` não aberto em vez de `is_open()`. O contador era incrementado mesmo sem arquivo de captura aberto.

Impacto:

- **não afeta ingest**;
- **não afeta PlaySDK decode**;
- **não afeta filas/drops/retries/failures**;
- **não afeta o PASS dos quatro streams**;
- invalida apenas `CaptureWritten/CaptureBytes` deste cenário.

Correção aplicada em commit posterior ao teste: captura só é contabilizada quando `is_open()` é verdadeiro e write/flush são verificados.

## 5. Conclusão

- 4 streams Extra simultâneos no mesmo processo: PASS;
- 4 RealPlay concorrentes na mesma sessão NetSDK: PASS no cenário;
- 4 PlaySDK ports concorrentes: PASS;
- bounded queues: sem drops;
- teardown: filas zeradas;
- nenhum retry/failure de input.

Próximo cenário: 4 streams Principal simultâneos, após rebuild com a correção de telemetria.

## Adendo — topologia real do NVR

Após o teste, o operador informou que este gravador possui **apenas 3 câmeras conectadas**.

Consequência para a interpretação:

- o cenário de 4 canais continua válido para provar **4 handles RealPlay + 4 portas PlaySDK concorrentes no mesmo processo**;
- ele **não é válido como benchmark de carga de 4 câmeras reais simultâneas**;
- pelo menos um dos canais testados corresponde a canal vazio/sem câmera, podendo o NVR fornecer stream de ausência de sinal/placeholder;
- métricas agregadas de bitrate/FPS deste cenário não devem ser usadas para extrapolar desempenho de 4 câmeras reais;
- benchmarks canônicos de carga devem usar apenas canais com câmera real ou outro NVR com quantidade suficiente de câmeras ativas.

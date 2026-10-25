# SENTRIUM-CPP-Foundation-004 — 4x PRINCIPAL MULTI-STREAM VALIDATION

**Registro:** F004-MEDIA-REAL-004  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-004 — Media Lab  
**Data:** 25/10/2026  
**Estado:** **PASS FUNCIONAL / TELEMETRIA DE CAPTURA NÃO CANÔNICA**

## 1. Cenário

Um único processo `sentrium-media-lab`, um único login NetSDK no NVR e quatro canais simultâneos:

- canais: 0, 1, 2, 3;
- stream: Principal;
- duração nominal: 10 s;
- backend: PlaySDK headless;
- transporte: NetSDK LAN.

## 2. Observação de build

O comando de build foi digitado como `sentrium-debugm` e falhou por preset inexistente. Em seguida foi executado o binário anterior, identificado por:

`BuildId: source-c26e89423adead61`

Portanto:

- a prova funcional de 4 streams Principal é válida para o binário c26e894;
- a correção posterior da telemetria de captura não estava presente;
- `CaptureWritten/CaptureBytes` deste teste são inválidos e não devem ser usados;
- este resultado não é o benchmark canônico final da F004.

## 3. Resultado agregado funcional

- Result: PASS;
- ChannelCount: 4;
- wall time: ~10.95 s;
- packets: 653;
- bytes: 3,880,762;
- throughput agregado observado: ~2.83 Mb/s;
- decoded callbacks: 596;
- queue drops: 0;
- max queue HWM: 4;
- input retries: 0;
- input failures: 0.

## 4. Resultado por canal

### Canal 0
- PASS;
- 1280×720;
- 15 fps declarados;
- ~14.90 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

### Canal 1
- PASS;
- 1280×720;
- 15 fps declarados;
- ~15.00 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

### Canal 2
- PASS;
- 1280×720;
- 15 fps declarados;
- ~14.80 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

### Canal 3
- PASS;
- 1280×720;
- 15 fps declarados;
- ~14.90 decoded fps;
- queue drops: 0;
- queue depth at stop: 0;
- input failures: 0.

## 5. Interpretação

Os quatro decoders mantiveram aproximadamente a cadência nominal de 15 fps por canal sem backlog ou falha de alimentação.

O campo agregado `DecodedFps=54.415` divide o total de frames pelo wall time que inclui a abertura e o fechamento concorrentes. Para avaliação de cadência de cada stream, usar os FPS individuais.

Os bitrates por canal diferiram bastante. Isso pode ocorrer por conteúdo/codificação VBR e não deve ser interpretado como diferença de capacidade do decoder sem inspeção adicional.

## 6. Conclusão

- 4 streams Principal simultâneos: PASS funcional;
- 4 RealPlay concorrentes na mesma sessão NetSDK: PASS;
- 4 PlaySDK ports concorrentes: PASS;
- bounded queues: zero drops;
- teardown: depth 0;
- alimentação PlaySDK: zero retries/failures;
- telemetria de captura: INVALID neste run por binário anterior.

Próxima ação: rebuild correto no HEAD atual e repetição curta do cenário para obter evidência canônica com telemetria corrigida.

# SENTRIUM-CPP-Foundation-004 — PRINCIPAL VS EXTRA VALIDATION

**Registro:** F004-MEDIA-REAL-002  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-004 — Media Lab  
**Data:** 25/10/2026  
**Estado:** **PASS — Principal + Extra / PlaySDK / NetSDK-LAN**

## 1. Objetivo

Validar que os streams Principal e Extra permanecem distintos no Media Lab e que ambos percorrem o pipeline real:

**NetSDK RealPlay → RAW/DHAV → bounded queue → PlaySDK headless → frame decodificado**

Mesmo equipamento Intelbras, mesmo canal lógico, mesmo transporte LAN.

## 2. Principal — resultado já validado

- resolução: 1280 × 720;
- frame rate declarado: 15 fps;
- decoded FPS observado: ~14.90;
- throughput comprimido: ~1.73 Mb/s;
- packets: 184;
- bytes: 2,159,842;
- queue drops: 0;
- input retries: 0;
- input failures: 0;
- HWM: 3.

## 3. Extra — resultado real

```json
{
  "Result": "PASS",
  "Foundation": "Foundation-004",
  "Version": "0.0.0-foundation004",
  "BuildId": "source-37a73b2144a0adfa",
  "Backend": "PlaySDK",
  "Transport": "NetSDK-LAN",
  "Stream": "Extra",
  "ChannelIndex": 0,
  "DurationSeconds": 10.0009,
  "Packets": 40,
  "Bytes": 212772,
  "MegabitsPerSecond": 0.170203,
  "QueueDropped": 0,
  "QueueHighWaterMark": 3,
  "QueueDepthAtStop": 0,
  "IgnoredPackets": 0,
  "InputRetries": 0,
  "InputFailures": 0,
  "DecodedCallbacks": 30,
  "DecodedFps": 2.99974,
  "LastWidth": 704,
  "LastHeight": 480,
  "LastFrameRate": 3,
  "LastFrameType": 3,
  "CaptureWritten": true,
  "CaptureBytes": 212772
}
```

## 4. Comparação observada

| Métrica | Principal | Extra |
|---|---:|---:|
| Resolução | 1280×720 | 704×480 |
| FPS declarado | 15 | 3 |
| Decode FPS observado | ~14.90 | ~3.00 |
| Throughput | ~1.73 Mb/s | ~0.17 Mb/s |
| Queue drops | 0 | 0 |
| Input retries | 0 | 0 |
| Input failures | 0 | 0 |
| HWM | 3 | 3 |

O Extra testado utiliza aproximadamente 9,8% do throughput comprimido observado no Principal desse mesmo canal e cerca de 20% da taxa de frames.

Isso é observação deste equipamento/configuração, não regra universal de produto.

## 5. Conclusão

- F004-AC-05 — Principal/Extra distintos: PASS.
- PlaySDK decodifica ambos os perfis: PASS.
- fila bounded sem backlog nos dois testes de 1 stream: PASS.
- enum oficial `DH_RType_Realplay_1` está presente no kit e foi validado em hardware real: PASS.

## 6. Próximo cenário

Executar 4 canais simultâneos no mesmo processo para medir escalabilidade real do mesmo backend e do mesmo tipo de stream.

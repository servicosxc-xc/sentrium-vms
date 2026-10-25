# SENTRIUM-CPP-Foundation-004 — FIRST REAL MEDIA VALIDATION

**Registro:** F004-MEDIA-REAL-001  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-004 — Media Lab  
**Data:** 25/10/2026  
**Estado:** **PASS — Principal / PlaySDK / NetSDK-LAN**

## 1. Caminho validado

**SENTRIUM VMS → NetSDK RealPlay → callback RAW/DHAV → bounded queue → PlaySDK headless → decoded frames**

Teste executado em equipamento Intelbras real por LAN, canal índice 0, stream Principal.

## 2. Resultado bruto reportado

```json
{
  "Result": "PASS",
  "Foundation": "Foundation-004",
  "Version": "0.0.0-foundation004",
  "BuildId": "source-ca48c57cec7a59ea",
  "Backend": "PlaySDK",
  "Transport": "NetSDK-LAN",
  "Stream": "Principal",
  "ChannelIndex": 0,
  "DurationSeconds": 10.0003,
  "Packets": 184,
  "Bytes": 2159842,
  "MegabitsPerSecond": 1.72782,
  "QueueDropped": 0,
  "QueueHighWaterMark": 3,
  "QueueDepthAtStop": 0,
  "IgnoredPackets": 0,
  "InputRetries": 0,
  "InputFailures": 0,
  "DecodedCallbacks": 149,
  "DecodedFps": 14.8995,
  "LastWidth": 1280,
  "LastHeight": 720,
  "LastFrameRate": 15,
  "LastFrameType": 3,
  "CaptureWritten": true,
  "CaptureBytes": 2159842
}
```

## 3. Interpretação

- ingest real: PASS;
- PlaySDK decode headless: PASS;
- 149 callbacks decodificados em ~10 s;
- decode observado: ~14.90 fps;
- frame rate declarado pelo PlaySDK: 15 fps;
- resolução observada: 1280 × 720;
- throughput comprimido observado: ~1.73 Mb/s;
- queue drops: 0;
- queue HWM: 3;
- input retries: 0;
- input failures: 0;
- ignored packets: 0;
- fila drenada no teardown: depth 0;
- captura RAW/DHAV: 2,159,842 bytes.

O decode observado acompanha de perto a taxa declarada de 15 fps, sem evidência de backlog no cenário de 1 stream.

## 4. Nota de BuildId

O BuildId persistido foi `source-ca48c57cec7a59ea`, correspondente ao configure que gerou o diretório de build antes dos commits posteriores de correção de compilação/teste.

Este resultado é válido como prova funcional do primeiro media path, mas **não deve ser tratado como evidência canônica final da F004**. Antes do benchmark final, será feito configure limpo para que BuildId e HEAD coincidam.

## 5. Segurança

Nenhuma credencial foi registrada nesta evidência.

A senha foi removida do ambiente após o teste.

## 6. Estado dos critérios

- F004-AC-04 — NetSDK RealPlay real por LAN entrega pacotes: PASS.
- F004-AC-06 — PlaySDK oracle decodifica pelo menos um stream real: PASS.
- F004-AC-08 — fila bounded observável: PASS no cenário.
- F004-AC-13 — ingest/decode separados por métricas: PASS para o slice atual.
- F004-AC-16 — nenhuma credencial persistida: PASS.
- F004-AC-18 — nenhuma alegação de ganho de performance: preservado.

## 7. Limites

Ainda não validado:

- stream Extra;
- 4/16 streams;
- render;
- FFmpeg;
- D3D11VA;
- CPU/memória/GPU benchmark canônico;
- Release;
- decisão ADR-005.

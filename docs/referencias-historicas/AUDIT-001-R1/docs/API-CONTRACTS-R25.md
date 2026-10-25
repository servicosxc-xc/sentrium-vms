# API/contratos r25

Inventário estático de **75 registros MapGet/Post/Put/Delete** mais mídia WebSocket via Map e hub SignalR. Não são 75 paths distintos nem uma especificação OpenAPI completa. [JSON de rotas](../artifacts/API-ENDPOINTS-R25.json). Tipos/DTOs estão em SharedContracts; símbolos e linhas em INVENTARIO-R25.json.

| Método | Rota | Evidência Program.cs |
|---|---|---|
| GET | `/evidence/{externalId}` | linha 119 |
| GET | `/api/v1/health/video` | linha 120 |
| GET | `/api/v1/health/evidence` | linha 121 |
| GET | `/api/v1/health` | linha 132 |
| GET | `/api/v1/health/host` | linha 143 |
| GET | `/api/v1/health/details` | linha 144 |
| GET | `/api/v1/runtime` | linha 190 |
| GET | `/api/v1/status` | linha 202 |
| GET | `/api/v1/intelbras/sdk` | linha 205 |
| GET | `/api/v1/intelbras/cloud` | linha 208 |
| GET | `/api/v1/devices` | linha 211 |
| GET | `/api/v1/device-configurations` | linha 214 |
| GET | `/api/v1/device-configurations/{deviceId}` | linha 217 |
| POST | `/api/v1/device-configurations` | linha 231 |
| PUT | `/api/v1/device-configurations/{deviceId}` | linha 254 |
| DELETE | `/api/v1/device-configurations/{deviceId}` | linha 282 |
| POST | `/api/v1/device-configurations/test` | linha 299 |
| GET | `/api/v1/devices/{deviceId}/tunnel` | linha 319 |
| POST | `/api/v1/devices/{deviceId}/channels/{channelIndex:int}/ptz/preset` | linha 333 |
| POST | `/api/v1/devices/{deviceId}/connect` | linha 350 |
| POST | `/api/v1/devices/{deviceId}/disconnect` | linha 365 |
| POST | `/api/v1/devices/{deviceId}/channels/{channelIndex:int}/stream-probe/start` | linha 380 |
| POST | `/api/v1/devices/{deviceId}/channels/{channelIndex:int}/stream-probe/stop` | linha 401 |
| GET | `/api/v1/devices/{deviceId}/channels/{channelIndex:int}/stream-probe` | linha 416 |
| GET | `/api/v1/stream-probes` | linha 431 |
| GET | `/api/v1/media/status` | linha 434 |
| GET | `/api/v1/ai/motion` | linha 437 |
| POST | `/api/v1/ai/motion/devices/{deviceId}/channels/{channelIndex:int}/start` | linha 440 |
| POST | `/api/v1/ai/intelligent/devices/{deviceId}/channels/{channelIndex:int}/start` | linha 460 |
| POST | `/api/v1/ai/motion/devices/{deviceId}/channels/{channelIndex:int}/stop` | linha 480 |
| GET | `/api/v1/ai/motion/devices/{deviceId}/channels/{channelIndex:int}` | linha 491 |
| GET | `/api/v1/ai/config/devices/{deviceId}/channels/{channelIndex:int}` | linha 502 |
| GET | `/api/v1/ai/rules` | linha 508 |
| GET | `/api/v1/ai/rules/devices/{deviceId}/channels/{channelIndex:int}` | linha 511 |
| POST | `/api/v1/ai/rules/devices/{deviceId}/channels/{channelIndex:int}` | linha 517 |
| DELETE | `/api/v1/ai/rules/devices/{deviceId}/channels/{channelIndex:int}/{ruleId:guid}` | linha 532 |
| PUT | `/api/v1/ai/config/devices/{deviceId}/channels/{channelIndex:int}` | linha 542 |
| GET | `/api/v1/ai/frame/devices/{deviceId}/channels/{channelIndex:int}` | linha 552 |
| GET | `/api/v1/events/history` | linha 563 |
| GET | `/api/v1/storage/retention` | linha 564 |
| PUT | `/api/v1/storage/retention` | linha 565 |
| GET | `/api/v1/storage/metrics` | linha 569 |
| GET | `/api/v1/ai/evidence` | linha 570 |
| GET | `/api/v1/ai/evidence/{eventId:guid}/thumbnail` | linha 573 |
| GET | `/api/v1/ai/evidence/{eventId:guid}/snapshot` | linha 578 |
| GET | `/api/v1/ai/evidence/{eventId:guid}/video` | linha 586 |
| GET | `/api/v1/lpr/master` | linha 595 |
| PUT | `/api/v1/lpr/master` | linha 596 |
| GET | `/api/v1/lpr/status` | linha 598 |
| GET | `/api/v1/lpr/engine` | linha 612 |
| POST | `/api/v1/lpr/engine/initialize` | linha 615 |
| GET | `/api/v1/lpr/readings` | linha 618 |
| GET | `/api/v1/lpr/attempts` | linha 621 |
| GET | `/api/v1/lpr/quality` | linha 624 |
| GET | `/api/v1/lpr/diagnostics` | linha 627 |
| POST | `/api/v1/lpr/counters/reset` | linha 628 |
| GET | `/api/v1/lpr/reviews` | linha 634 |
| GET | `/api/v1/lpr/readings/{readingId:guid}/review` | linha 637 |
| PUT | `/api/v1/lpr/readings/{readingId:guid}/review` | linha 643 |
| GET | `/api/v1/lpr/readings/{readingId:guid}/plate-crop` | linha 662 |
| GET | `/api/v1/lpr/profiles` | linha 670 |
| GET | `/api/v1/lpr/runtime` | linha 673 |
| GET | `/api/v1/lpr/runtime/{profileId:guid}` | linha 676 |
| GET | `/api/v1/lpr/recovery` | linha 688 |
| POST | `/api/v1/lpr/profiles/{profileId:guid}/sample` | linha 691 |
| GET | `/api/v1/lpr/profiles/{profileId:guid}` | linha 710 |
| POST | `/api/v1/lpr/profiles` | linha 722 |
| PUT | `/api/v1/lpr/profiles/{profileId:guid}` | linha 741 |
| DELETE | `/api/v1/lpr/profiles/{profileId:guid}` | linha 761 |
| GET | `/api/v1/media/ws` | linha 770 |
| GET | `/api/v1/integrations/muralha/config` | linha 905 |
| PUT | `/api/v1/integrations/muralha/config` | linha 908 |
| GET | `/api/v1/integrations/muralha/status` | linha 913 |
| GET | `/api/v1/integrations/muralha/history` | linha 916 |
| POST | `/api/v1/foundation/events/simulate` | linha 919 |
| WS GET upgrade | `/api/v1/media/ws` | app.Map(ApiRoutes.MediaWebSocket,...) |
| SignalR negotiate/transport | `/hubs/runtime` | app.MapHub<RuntimeHub>(HubRoutes.Runtime) |

## Semântica e compatibilidade

REST usa JSON web/camelCase por padrão, GUIDs, UTC/DateTimeOffset, enums conforme serializer atual (não converter silenciosamente para nomes), nulls e limites por endpoint. Evidence retorna BMP/JPEG/DAV binário; snapshot/crop/vídeo internos permitem Range onde configurado. Novo Gateway tem 401/403/404/503 e no-store/nosniff, bearer só no header. Erros de configuração/dispositivo usam 400/404/409 conforme handlers; DELETE pode retornar 204. Não padronizar tudo em 200 na v2.

Listagem de evidências: limit padrão 50/page1, filtros device/channel/from/until/type/search. LPR readings: limit200/page1/profile/plate, clamp no store. Histórico Muralha: limites e ordem estável; reviews e reset de counters não removem registros. Master PUT aguarda stop/reconcile. Device DTO expõe indicação HasPassword em vez de segredo; verificar todos os contratos na compatibilidade.

A mídia WebSocket exige handshake JSON com Protocol/Version/DeviceId/ChannelIndex/RequestedStream/EffectiveStream/StartedAtUtc/QueueCapacity. Cada mensagem binária preserva header XCVI v1 de 36 bytes little-endian: magic0–3, version4, flags5 (extra bit0), reserved6–7, channel8–11, dataType12–15, sequence16–23, timestampUTCms24–31, length32–35, payload comprimido intacto. Um callback/pacote não equivale a frame.

SignalR em /hubs/runtime publica RuntimeStatusChanged, EventPublished, AiDetectionsUpdated e EventActionRequested; ao conectar envia status atual. Client usa retry/reconnect. Drogon WebSocket não implementa automaticamente protocolo SignalR, negotiate, handshake ou framing. Proposta: preservar payloads; definir transporte v2 explicitamente versionado e, se Client r25 precisar falar com Server v2, fornecer adaptador SignalR testado ou manter control-plane bridge temporário separado. Nunca anunciar drop-in compatibility antes desse gate.

Grupos de DTOs: Devices/connection/channel/PTZ; Events/evidence/regras/detections; LPR profile/master/runtime/recovery/readings/reviews/attempts/quality/queue; Integrations/Muralha config/mapping/history/status/LAP; Runtime/health/host/retention; Actions/invocation/target. A matriz de arquivos cobre todos. Golden API snapshots devem incluir request/resposta/status/query/ordem/ranges, não apenas nome de endpoint.

Não houve chamada HTTP à instância operacional nesta auditoria; rotas de diagnóstico não foram usadas para consultar credenciais ou dispositivos.
# SENTRIUM-CPP-Foundation-003 — REAL DEVICE VALIDATION

**Registro:** F003-REAL-DEVICE-001  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-003 — Device Foundation  
**Data:** 25/10/2026  
**Estado:** **PASS — LAN / NetSDK**

## Ambiente validado

Primeira validação real do novo cliente C++ contra equipamento Intelbras em rede local.

Caminho validado:

**SENTRIUM VMS → NetSDK nativo C++ → equipamento Intelbras por IP/LAN**

Cloud/P2P não participou deste teste.

## Resultado observado

- login NetSDK: PASS;
- estado exibido: Connected;
- enumeração de canais: PASS;
- árvore da UI populada a partir do retorno real do equipamento;
- quantidade observada: 18 canais;
- disconnect: PASS;
- reconnect: PASS;
- múltiplos ciclos disconnect/connect: PASS;
- fechamento normal do cliente após os ciclos: PASS;
- nenhuma credencial registrada nesta evidência.

## Limites

Este teste não valida:

- Cloud/P2P;
- libt2u;
- Direct/Relay;
- PlaySDK;
- RealPlay;
- vídeo;
- decode;
- gravação;
- playback;
- PTZ;
- eventos;
- IA;
- LPR.

## Conclusão

**F003-AC-14 — PASS**  
**F003-AC-15 — PASS**

A conexão Intelbras por IP/LAN usando NetSDK nativo C++ está funcional no ambiente real testado.

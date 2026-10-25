# AUDIT-001 — addendum Intelbras / Cloud

Data: 23/10/2026. Revisão documental posterior à entrega original. Status: **GO WITH BLOCKERS**. CPP-001 NÃO iniciada.

## Proveniência e limite da evidência

Fonte desta revisão: relato técnico fornecido pelo usuário no anexo `4a46d225-3e38-4636-90d5-6b265342c575/Pasted text.txt`, contendo resultados de inspeção de pacote, reflexão/IL e observações de runtime do SIM Next. As descobertas são incorporadas com essa proveniência: esta revisão não reexecutou captura, IL, DLLs, probe ABI ou sessão Cloud. Não foram fornecidos aqui arquivos brutos/hashes do novo kit nem PCAP/IL; logo não se atribui a esta auditoria reprodução independente ou cadeia de custódia desses materiais. Os hashes PE antigos continuam descrevendo somente a árvore r25, sem serem atribuídos ao kit localizado.

## 1. Caminho nativo C++ comprovado no levantamento fornecido

Pacote localizado/inspecionado: `General_NetSDK_3.050_PlaySDK_3.042`.

| Componente | Artefatos x64 identificados | Classificação atual |
|---|---|---|
| NetSDK | dhnetsdk.h, dhnetsdk.lib, dhconfigsdk.lib, DLLs correspondentes e exemplos nativos | disponibilidade C++ resolvida tecnicamente |
| PlaySDK | dhplay.h, dhplay.lib, DLLs correspondentes e exemplos nativos | disponibilidade C++ resolvida tecnicamente |

Removido o blocker genérico de SDK C++ indisponível. O pacote antigo ainda exige compatibilidade/paridade com equipamentos e runtime SIM Next atuais, validação do subset utilizado e termos de uso/redistribuição. Presença do kit não autoriza copiar binários para o produto.

## 2. Cloud nativo e bootstrap

Cadeia observada: `SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll`. Reflexão/IL confirma wrapper .NET/PInvoke; .NET não é dependência arquitetural obrigatória da futura implementação C++.

Exports principais identificados: `t2u_init`, `t2u_init_v2`, `t2u_set_callback`, `t2u_add_port`, `t2u_add_port_ex`, `t2u_add_port_v3`, `t2u_del_port`, `t2u_port_status`, `t2u_get_rate`, `t2u_query`, `t2u_query_ex`, `t2u_search`, `t2u_search_dvr`, `t2u_status`, `t2u_exit`, `t2u_set_port_range`. Priorizar o caminho SIM Next observado; não priorizar `t2u2_*` nem variantes UDP específicas inicialmente. Exports conhecidos não bastam para fechar assinaturas/ABI/licença.

IL: `t2u_init(server, 1250, key)`. Tráfego real observado: UDP remoto/1250, múltiplos endpoints. Servidor/configuração/credential devem vir oficialmente do SDK/Intelbras. Não hardcodear IP e não extrair, documentar ou reutilizar credencial privada/obfuscada do SIM Next. Esta revisão não contém valor de credential.

## 3. Criação de túnel e lifetime

`VVEyeAdapter.AddSDKPort(deviceId, uuid, remotePort, localPort)` chama `t2u_add_port_v3(...)`, espera 100 ms e consulta `t2u_port_status(...)`; repete enquanto status == 0 por aproximadamente até 100 tentativas. status == 1 é sucesso; falha remove a porta e retorna 0. O retorno de add_port_v3 é tratado como porta local efetiva do túnel. A futura API deve preservar distinção entre retorno de criação, disponibilidade e diagnóstico, com deadline/cancelamento fora do HTTP loop.

Caso isolado observado: listener `0.0.0.0:31621`; controle loopback permanece sem vídeo; abrir vídeo adiciona conexão TCP efêmera a `127.0.0.1:31621`; fechar remove a conexão RealPlay, preservando listener/controle; reabrir cria nova conexão efêmera. **31621 é exemplo observado, nunca constante de produto**. Não interpretar LISTEN em 0.0.0.0 como bind exclusivamente loopback: exposição/firewall/bind configurável exigem avaliação de segurança futura, sem executar probe agora.

Conclusão: **Cloud tunnel lifetime != RealPlay lifetime**. Túnel pertence ao dispositivo/sessão Cloud e é compartilhado pelas sessões NetSDK: login/control, configuração, PTZ/eventos e RealPlay Principal/Extra. Não criar/destruir P2P por tile. Manager deve manter referências de controle e consumidores; último RealPlay não implica último usuário do túnel.

## 4. Estado Direct/Relay

RelayConnection(port) consulta `t2u_port_status(port, stat)` e retorna verdadeiro exatamente se `stat.proxy == 1`: Relay (amarelo). **connected && proxy != 1**: Direct P2P (azul). Sem conexão/estado válido: desconectado/desconhecido; não pintar Direct apenas por proxy default. P2pIsActive(deviceId) consulta presença no dictionary, não prova Direct nem conectividade; preferir `hasCloudTunnel()` para essa semântica.

## 5. ABI provável — pendente de validação

Descrições a seguir são registro documental, não headers reconstruídos nem código de produto.

| Estrutura | Layout relatado | Tamanho provável x64 |
|---|---|---|
| T2uNetStat | char ip[20]; int32 port; int32 proxy; float lost_rate; int32 bandwidth; int32 remote_nattype; int32 local_nattype; char ip6[40]; int32 remote_version; Sequential/Pack8/ANSI | 88 bytes |
| T2uNetRate | int64 total_recv, total_send; float cur_recv_rate, cur_send_rate, cur_rate, avg_recv_rate, avg_send_rate, avg_rate; int32 time_sec; Pack8 | 48 bytes |
| T2uCallback | OnConnect e OnDisconnect; ponteiros de callback Cdecl void(uint16 localPort, int32 useTime) | 16 bytes |

Nenhum tamanho é definitivo sem sizeof/static_assert/offsets e probe contra DLL correspondente, em etapa futura autorizada. Validar duração do callback, threading, strings, ownership, signedness, retorno e convenção das demais funções com contrato oficial. Não descarregar DLL enquanto callback/job/túnel estiver ativo.

## 6. Limites da captura

SIM Next tinha aproximadamente 155 dispositivos cadastrados. Tráfego UDP global agrega sessões. Não atribuir IP remoto específico à câmera isolada nem chamar todo endereço 38.250.x.x de infraestrutura/peer. Esta revisão não registra esses IPs operacionais. Somente UDP/1250 observado, arquitetura, conexões locais isoladas e lifetime descritos sustentam as conclusões acima. Testar NAT/reconexão/failover é pendência, não conclusão da captura.

## 7. Arquitetura candidata — sem implementação

`xc_cloud/`: T2uLibrary (LoadLibraryW/GetProcAddress e validação de exports); T2uRuntime (init/callbacks/exit, lifetime process-wide); CloudTunnel (device/uuid, remotePort/localPort/status, Direct/Relay, NAT/loss/bandwidth, destroy/recovery); CloudTunnelManager (um túnel por dispositivo quando possível, refcount/lifetime, recovery/reconnect/telemetry).

`xc_intelbras/`: NetSDK nativo, login em 127.0.0.1:CloudTunnel.localPort, canais/PTZ/eventos/RealPlay. Consumir endpoint/lease por interface; não conhecer implementação T2U. xc_cloud não depende de UI nem de NetSDK. Implementações falsas permitem scaffold sem DLL e sem SIM Next. Runtime process-wide não implica mutex global em todas as operações: política depende do contrato de thread-safety. Recovery invalida gerações/sessões NetSDK e recria endpoint sem uso de lease expirado.

## 8. Blockers antes/depois

Resolvido tecnicamente: disponibilidade NetSDK/PlaySDK C++; DLL libt2u e exports principais identificados; caminho Cloud → T2U → localhost → NetSDK comprovado no levantamento; Direct/Relay via stat.proxy; ausência de obrigação arquitetural .NET.

| ID | Blocker real remanescente | Gate |
|---|---|---|
| IB-01 | redistribuição libt2u.dll | autorização/licença explícita |
| IB-02 | bootstrap/configuração/credential | fornecimento oficial, configuração sem hardcode/segredo em log |
| IB-03 | uso standalone fora do SIM Next | confirmação Intelbras, contrato e dependências suportadas |
| IB-04 | Windows limpo sem SIM Next | teste real reproduzível sem dependência da instalação |
| IB-05 | reconexão e P2P ↔ Relay | teste de falhas/failover e telemetria, sem prometer continuidade não demonstrada |
| IB-06 | SDKs antigos versus ambiente atual | matriz firmware/runtime/codec, ABI e paridade NetSDK/PlaySDK |

São gates da integração Intelbras/Cloud pronta para produção. **Não impedem Foundation desacoplada futura**, desde que preserve interfaces xc_cloud/xc_intelbras, não hardcodeie credenciais, não redistribua DLL sem autorização e não aceite instalação SIM Next como solução final. Esta revisão não inicia essa Foundation. Demais bloqueios da auditoria permanecem.

Ver [arquitetura](docs/ARQUITETURA-CPP-PROPOSTA.md), [SDK](docs/SDK-INTELBRAS-NATIVO-NECESSARIO.md), [riscos](docs/RISCOS-E-BLOQUEIOS.md), [ADR-010](docs/adrs/ADR-010-interoperability-intelbras.md) e [resultado](AUDIT-001-RESULTADO.md).
# SENTRIUM VMS — Intelbras Cloud / P2P (T2U)
## Descobertas técnicas consolidadas

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Data de consolidação:** 25/10/2026  
**Status:** **EVIDENCE CONSOLIDATION — NO NEW ASSUMPTIONS**  
**Origem:** legado XC Video IA, probes Cloud/P2P, reflexão/IL e AUDIT-001-INTELBRAS-CLOUD-ADDENDUM  
**Finalidade:** servir como baseline factual para a futura Foundation de conectividade Cloud/P2P.

> Este documento registra somente o que já foi observado, testado ou identificado nos materiais existentes.  
> Onde a evidência é apenas provável ou documental, isso é explicitamente indicado.  
> Não contém credencial secreta, não autoriza redistribuição de DLLs e não converte endpoints históricos em contrato público.

---

# 1. Resumo executivo

A principal descoberta é que o **Cloud/P2P usado pelo SIM Next não é um login Cloud direto do NetSDK**.

Existe uma camada anterior de transporte, baseada em T2U, que:

1. recebe o identificador Cloud/dispositivo;
2. executa sinalização/bootstrap;
3. estabelece **Direct P2P ou Relay**;
4. cria um listener TCP local;
5. entrega ao restante do sistema um endpoint equivalente a:

`127.0.0.1:<porta-local-dinâmica>`

Depois disso, o **NetSDK trabalha por TCP normal**, sem precisar conhecer a mecânica Cloud/P2P.

A cadeia observada foi:

```text
Cloud UUID / dispositivo
        ↓
SIMNext.P2PSDK.dll
        ↓
Libt2uWrapper.dll
        ↓
libt2u.dll
        ↓
Direct P2P ou Relay
        ↓
listener TCP local
        ↓
127.0.0.1:<porta dinâmica>
        ↓
NetSDK TCP normal
        ↓
login / canais / PTZ / eventos / RealPlay
```

O transporte efetivo está em uma biblioteca **nativa**. O wrapper .NET apenas chama essa camada. Portanto, **.NET não é requisito arquitetural obrigatório** para a implementação futura do SENTRIUM em C++.

---

# 2. Descoberta que mudou a estratégia

O teste inicial do legado tentou usar o Cloud diretamente pelo NetSDK.

Essa abordagem mostrou-se insuficiente:

> passar somente o Serial/Cloud ID para o login Cloud/P2P do NetSDK não reproduzia o comportamento do SIM Next.

A inspeção do SIM Next e do tráfego mostrou uma etapa anterior de:

- bootstrap;
- rendezvous;
- NAT traversal;
- P2P/Relay;
- criação de túnel local.

A partir disso, a estratégia experimental do XC Video IA v0.4/v0.4.2 mudou para:

```text
Cloud ID
  ↓
T2U
  ↓
P2P / Relay
  ↓
TCP local
  ↓
NetSDK
```

---

# 3. Cadeia de componentes observada

## 3.1 Cadeia principal

Foi identificada a sequência:

```text
SIMNext.P2PSDK.dll
        ↓
Libt2uWrapper.dll
        ↓
libt2u.dll
```

A reflexão/IL mostrou que o wrapper .NET utiliza P/Invoke para chegar à biblioteca nativa.

## 3.2 Componentes e versões observadas

| Componente | Versão observada | Papel identificado |
|---|---:|---|
| `SIMNext.P2PSDK.dll` | 1.25.4.1203 | Wrapper de alto nível usado pelo SIM Next |
| `SIMNext.Core.dll` | 1.25.4.1203 | Dependência do P2PSDK |
| `SIMNext.Model.dll` | 1.25.4.1203 | Dependência do P2PSDK |
| `Libt2uWrapper.dll` | 1.3.0.0 | Wrapper .NET/PInvoke para T2U nativo |
| `libt2u.dll` | versão de assembly não exposta | Transporte T2U nativo |
| `NLog.dll` | asm 5.0.0.0 / file 5.0.1.1159 | Dependência observada no host |

No Cloud Host Probe 003 também foram registrados prefixes de SHA-256, úteis apenas para identificar o conjunto testado:

- `SIMNext.P2PSDK.dll`: `568563393c6a...`
- `SIMNext.Core.dll`: `66547e8ff53c...`
- `SIMNext.Model.dll`: `585f661dda96...`
- `Libt2uWrapper.dll`: `0dbef79ccdc2...`
- `libt2u.dll`: `528b8289c6f5...`
- `NLog.dll`: `68936e2536bc...`

Esses prefixes não substituem hashes completos/canônicos de distribuição.

---

# 4. API observada no `SIMNext.P2PSDK`

Por reflexão, o `VVEyeAdapter` apresentou:

```text
Int32  AddSDKPort(
    Int64 deviceId,
    String uuid,
    UInt16 remotePort,
    UInt16 localPort)

Boolean P2pIsActive(Int64 deviceId)

Boolean RelayConnection(UInt16 port)

Void DeleteSDKPort(
    Int64 deviceId,
    UInt16 port)

Void Cleanup()
```

Também foi observado o construtor:

```text
VVEyeAdapter(
    ITelemetryReportService TelemetryReportService,
    ISystemSettingsService systemSettingsService)
```

Nos probes de compatibilidade, foi possível instanciar:

```text
VVEyeAdapter(null, null)
```

e estabelecer a sessão Cloud.

---

# 5. Identificador interno / DeviceKey

Nos probes, o identificador Cloud foi mantido mascarado nos logs como:

```text
***0798
```

Foi observado um `Int64 deviceId` derivado/utilizado pelo wrapper:

```text
539645815473672516
```

Esse valor foi passado ao `AddSDKPort` e usado posteriormente em:

- `P2pIsActive(deviceId)`;
- `DeleteSDKPort(deviceId, port)`.

O mecanismo formal de geração/contrato desse `deviceId` ainda não é tratado neste documento como API oficial.

---

# 6. Porta de serviço remota

No equipamento testado, a **Porta de Serviço** Cloud era:

```text
37000
```

Ela foi passada ao:

```text
AddSDKPort(..., remotePort=37000, ...)
```

Uma descoberta importante do legado foi:

> a Porta de Serviço Cloud não deve ser assumida como `37777`.

Ela deve vir da configuração/cadastro correspondente do dispositivo.

---

# 7. API nativa T2U identificada

Entre os exports encontrados em `libt2u.dll`:

- `t2u_init`
- `t2u_init_v2`
- `t2u_set_callback`
- `t2u_add_port`
- `t2u_add_port_ex`
- `t2u_add_port_v3`
- `t2u_del_port`
- `t2u_port_status`
- `t2u_get_rate`
- `t2u_query`
- `t2u_query_ex`
- `t2u_search`
- `t2u_search_dvr`
- `t2u_status`
- `t2u_exit`
- `t2u_set_port_range`

A existência desses exports foi identificada.

Isso **não fecha automaticamente**:

- assinatura ABI final;
- calling convention;
- ownership;
- lifetime de buffers;
- threading;
- licença de uso/redistribuição.

---

# 8. Bootstrap observado

A inspeção de IL documentou a chamada conceitual:

```text
t2u_init(server, 1250, key)
```

Também foi observado tráfego UDP real envolvendo a porta remota:

```text
1250/UDP
```

Foram vistos múltiplos endpoints.

## 8.1 Conclusão sobre bootstrap

O bootstrap **não deve ser hardcoded** com base nas capturas.

Servidor, configuração e credential precisam vir de uma fonte oficial/suportada.

A auditoria determinou explicitamente:

- não extrair credential privada/obfuscada do SIM Next;
- não documentar seu valor;
- não copiar credential;
- não hardcodear credential.

---

# 9. Endpoints históricos observados — valor probatório limitado

Em uma fase antiga da investigação, durante packet capture/pktmon, foram registrados tráfegos compatíveis com o processo Cloud, incluindo exemplos como:

- `38.250.250.24:1252` — observado como rendezvous experimental na fase v0.4;
- candidatos Direct envolvendo endereço local e endereço público;
- tráfego Relay observado em outro endpoint `38.250.x.x`.

Entretanto, a investigação posterior mostrou que o SIM Next daquele ambiente possuía aproximadamente:

```text
155 dispositivos cadastrados
```

Portanto, capturas globais agregavam várias sessões.

## 9.1 Regra consolidada

Esses IPs históricos:

- **não são contrato público**;
- **não devem ser hardcoded**;
- **não podem ser atribuídos arbitrariamente a um único dispositivo**;
- **não provam topologia estática da infraestrutura Intelbras**.

A auditoria final preservou como evidência segura:

- uso de T2U;
- tráfego UDP;
- porta 1250 observada;
- múltiplos endpoints;
- listener local;
- comportamento de lifetime.

---

# 10. Sequência interna do `AddSDKPort`

A análise do `VVEyeAdapter.AddSDKPort` documentou o seguinte comportamento:

1. recebe:
   - `deviceId`;
   - `uuid`;
   - `remotePort`;
   - `localPort`;

2. chama:

```text
t2u_add_port_v3(...)
```

3. aguarda aproximadamente:

```text
100 ms
```

4. consulta:

```text
t2u_port_status(...)
```

5. enquanto:

```text
status == 0
```

o wrapper continua consultando;

6. a lógica observada realizava aproximadamente até:

```text
100 tentativas
```

7. quando:

```text
status == 1
```

a sessão é tratada como estabelecida;

8. em falha:
   - a porta é removida;
   - a operação retorna `0`;

9. em sucesso:
   - o retorno é tratado como a **porta TCP local efetiva** do túnel.

---

# 11. Porta local e listener TCP

O túnel T2U expõe uma porta TCP local.

O endpoint consumido pelo NetSDK passa a ser conceitualmente:

```text
127.0.0.1:<porta-dinâmica>
```

Portas observadas em diferentes probes incluem:

- 59143
- 59153
- 59163
- 58803
- 58843
- 58852
- 37792
- 37799
- 37806
- 38584

Esses números são **exemplos de runtime**.

Nenhum deles deve virar constante de produto.

---

# 12. Relação T2U ↔ NetSDK

A arquitetura comprovada pode ser resumida assim:

```text
CloudTunnel.localPort = porta dinâmica

NetSDK:
    host = 127.0.0.1
    port = CloudTunnel.localPort
```

Depois que o túnel está pronto, o NetSDK pode executar o fluxo normal:

- login;
- descoberta de canais;
- configuração;
- PTZ;
- eventos;
- RealPlay Principal;
- RealPlay Extra.

## 12.1 Descoberta importante

O NetSDK **não precisa implementar o Cloud**.

Ele precisa apenas receber um endpoint TCP funcional.

Isso desacopla:

```text
Cloud/T2U
```

de:

```text
NetSDK/protocolo do equipamento
```

---

# 13. Lifetime do túnel — comportamento comprovado

Foi observado um caso isolado com listener:

```text
0.0.0.0:31621
```

O comportamento observado foi:

1. o túnel permaneceu aberto sem nenhuma câmera sendo visualizada;
2. existia uma sessão/controle persistente associada à porta local;
3. abrir RealPlay criou uma conexão TCP adicional para:
   `127.0.0.1:31621`;
4. fechar RealPlay removeu essa conexão adicional;
5. o listener e a sessão Cloud permaneceram;
6. reabrir o vídeo criou nova conexão efêmera.

## 13.1 Conclusão comprovada

> **CloudTunnel lifetime != RealPlay lifetime**

O túnel pertence ao:

```text
dispositivo / sessão Cloud
```

e não ao:

```text
tile / janela de vídeo
```

O fechamento do último RealPlay não prova que o último consumidor do túnel terminou.

O mesmo túnel pode atender operações NetSDK de:

- login/control;
- configuração;
- PTZ;
- eventos;
- RealPlay Principal;
- RealPlay Extra.

## 13.2 Observação de segurança

O listener `0.0.0.0:31621` foi apenas um caso observado.

`LISTEN 0.0.0.0` não deve ser interpretado como bind exclusivamente loopback.

Avaliação futura ainda precisa verificar:

- exposição;
- firewall;
- política de bind;
- escopo do listener.

---

# 14. Direct P2P versus Relay

Foi identificado que o método:

```text
RelayConnection(port)
```

consulta:

```text
t2u_port_status(port, stat)
```

No comportamento legado investigado:

```text
stat.proxy == 1
```

representa:

```text
Relay
```

Quando a conexão é válida e:

```text
stat.proxy != 1
```

a interpretação é:

```text
Direct P2P
```

## 14.1 Matriz de estado

| Condição | Interpretação |
|---|---|
| conectado + `proxy == 1` | Relay |
| conectado + `proxy != 1` | Direct P2P |
| sem conexão/estado válido | desconectado/desconhecido |
| `P2pIsActive == true` | túnel registrado/presente; **não prova Direct** |

## 14.2 `P2pIsActive` não significa Direct P2P

A investigação mostrou que:

```text
P2pIsActive(deviceId)
```

consulta a presença/registro do túnel no wrapper.

Portanto:

> `P2pIsActive=True` não deve ser apresentado como “Direct P2P”.

Ele também não substitui a consulta do estado efetivo do transporte.

## 14.3 O que foi efetivamente testado

Nos probes reproduzidos e consolidados neste documento:

```text
Relay=True
```

foi repetidamente confirmado.

A lógica de identificação de Direct foi encontrada no wrapper/status, porém este documento **não declara que uma sessão Direct P2P foi comprovada com sucesso no ambiente testado**.

---

# 15. Cloud Host / TFM Probe 003

Foi executada uma matriz para verificar se o funcionamento dependia do tipo de host .NET.

Todos os testes foram:

- .NET 10.0.12;
- x64;
- mesmo dispositivo Cloud;
- remote/service port 37000.

## 15.1 Perfil desktop

```text
LocalPort      : 59143
P2pIsActive    : True
Relay          : True
TCP local      : True
Elapsed        : 3096 ms
Outcome        : SUCCESS
```

## 15.2 Perfil windows

```text
LocalPort      : 59153
P2pIsActive    : True
Relay          : True
TCP local      : True
Elapsed        : 3092 ms
Outcome        : SUCCESS
```

## 15.3 Perfil plain

```text
LocalPort      : 59163
P2pIsActive    : True
Relay          : True
TCP local      : True
Elapsed        : 3093 ms
Outcome        : SUCCESS
```

## 15.4 Conclusão do Probe 003

O P2PSDK estabeleceu o túnel em todos os perfis testados.

Isso reduziu a hipótese de que o Cloud dependia especificamente do host WindowsDesktop.

---

# 16. Isolation Probe 002

Uma investigação separou o Cloud de diferentes níveis de inicialização do NetSDK.

Foram registrados como sucesso:

- `p2p-only`;
- `netsdk-load`;
- `netsdk-init`.

Portas observadas nessa série:

- 58803
- 58843
- 58852

com:

```text
P2pIsActive=True
Relay=True
TCP local=True
```

## 16.1 Conclusão do Probe 002

A simples presença/carregamento/inicialização do NetSDK **não era a causa** da falha Cloud que estava sendo investigada naquela etapa.

---

# 17. Exact Integration Probe 004 r2

Foi executada uma matriz com três cenários.

## 17.1 Stage A — Cloud isolado

```text
Stage          : cloud-only
Success        : True
State          : READY
LocalPort      : 37792
ConnectionMode : Cloud/Relay
```

Tempo de abertura:

aproximadamente 3,03 s.

---

## 17.2 Stage B — runtime NetSDK real + Cloud

Antes do Cloud foi inicializado o:

```text
IntelbrasNetSdkRuntime
```

Resultado:

```text
Stage          : exact-runtime
Success        : True
State          : READY
LocalPort      : 37799
ConnectionMode : Cloud/Relay
```

Novamente, o túnel foi estabelecido normalmente.

---

## 17.3 Stage C — ASP.NET/Kestrel + runtime + Cloud

O probe foi executado dentro de um host ASP.NET/Kestrel.

Endpoint local do probe:

```text
http://127.0.0.1:5099/probe
```

Resultado:

```text
HTTP           : 200 OK
success        : true
localPort      : 37806
connectionMode : Cloud/Relay
state          : READY
```

Tempo da request:

```text
3107.0553 ms
```

## 17.4 Conclusão do Probe 004

Foram bem-sucedidos:

- Cloud isolado;
- Cloud com NetSDK runtime inicializado;
- Cloud dentro de ASP.NET/Kestrel.

Isso reforçou que o problema investigado anteriormente não era simplesmente causado por:

- NetSDK inicializado;
- thread pool;
- Kestrel;
- host ASP.NET.

---

# 18. Repetibilidade observada

Em múltiplos probes o padrão ficou próximo de:

```text
~3,0 a 3,2 segundos
```

para o estabelecimento do túnel Relay.

Foram observadas diversas portas locais diferentes, mas com o mesmo resultado lógico:

```text
AddSDKPort retornou porta local
P2pIsActive=True
Relay=True
TCP local aberto
State=READY
```

Essa repetibilidade é uma das evidências mais fortes do comportamento do wrapper legado.

---

# 19. ABI provável — ainda não validada

A auditoria registrou layouts prováveis.

Eles são **registro documental**, não headers definitivos.

| Estrutura | Layout relatado | Tamanho provável x64 |
|---|---|---:|
| `T2uNetStat` | `char ip[20]; int32 port; int32 proxy; float lost_rate; int32 bandwidth; int32 remote_nattype; int32 local_nattype; char ip6[40]; int32 remote_version; Sequential/Pack8/ANSI` | 88 bytes |
| `T2uNetRate` | `int64 total_recv, total_send; float cur_recv_rate, cur_send_rate, cur_rate, avg_recv_rate, avg_send_rate, avg_rate; int32 time_sec; Pack8` | 48 bytes |
| `T2uCallback` | `OnConnect` e `OnDisconnect`; callbacks Cdecl `void(uint16 localPort, int32 useTime)` | 16 bytes |

## 19.1 Ainda precisa ser validado

Antes de qualquer binding C++ de produção:

- `sizeof`;
- offsets;
- alignment;
- calling convention;
- signedness;
- strings;
- ownership;
- callback lifetime;
- callback threading;
- unload da DLL;
- retorno das demais funções.

A DLL não deve ser descarregada enquanto houver:

- callback ativo;
- job ativo;
- túnel ativo.

---

# 20. O que está comprovado

## 20.1 Arquitetura

**Comprovado/identificado:**

```text
Cloud → T2U → Direct/Relay → TCP local → NetSDK
```

## 20.2 Wrapper

Foram identificados:

- `AddSDKPort`;
- `P2pIsActive`;
- `RelayConnection`;
- `DeleteSDKPort`;
- `Cleanup`.

## 20.3 Transporte local

Foi comprovado que o túnel entrega uma porta TCP local utilizável.

## 20.4 Relay

O modo Relay foi comprovado em múltiplos probes.

## 20.5 Lifetime

Foi comprovado que:

```text
CloudTunnel lifetime != RealPlay lifetime
```

## 20.6 Independência em relação ao NetSDK

O Cloud pôde ser aberto:

- sem NetSDK;
- após carregar NetSDK;
- após inicializar NetSDK;
- com o runtime real;
- dentro de host ASP.NET/Kestrel.

---

# 21. O que NÃO está comprovado

Os itens abaixo continuam sem prova suficiente para produção:

## 21.1 Direct P2P efetivamente estabelecido

A lógica de classificação foi identificada.

Os probes consolidados confirmaram **Relay**.

Não há, neste conjunto, prova suficiente para declarar um Direct P2P real como validado.

## 21.2 Standalone sem SIM Next

Os testes antigos carregaram componentes existentes na instalação local do SIM Next.

Ainda falta provar uma distribuição independente e suportada.

## 21.3 Windows limpo

Ainda falta teste real em uma máquina sem SIM Next instalado.

## 21.4 Bootstrap oficial

Ainda não existe contrato oficial consolidado para:

- server;
- credential;
- atualização desses dados;
- rotação;
- fallback.

## 21.5 Failover

Não foi comprovado de forma reproduzível:

```text
Direct → Relay
Relay → Direct
queda → recovery
troca de rede/NAT
```

## 21.6 ABI C++ definitiva

Os exports são conhecidos.

As assinaturas/layouts finais ainda precisam de validação.

## 21.7 Nova implementação C++

O SENTRIUM C++ ainda não possui seu próprio CloudTunnel funcional.

Tudo que está descrito aqui é baseline de conhecimento para a futura implementação.

---

# 22. Blockers reais de produção

| ID | Blocker | Gate necessário |
|---|---|---|
| IB-01 | redistribuição de `libt2u.dll` | licença/autorização explícita |
| IB-02 | bootstrap/configuração/credential | fornecimento oficial e configuração segura |
| IB-03 | uso standalone fora do SIM Next | confirmação formal de dependências suportadas |
| IB-04 | Windows limpo sem SIM Next | teste reproduzível |
| IB-05 | reconexão / P2P ↔ Relay | testes de falha, failover e telemetria |
| IB-06 | SDK/runtime/firmware atuais | matriz de compatibilidade e validação ABI |

Esses blockers não invalidam as descobertas.

Eles impedem declarar o Cloud pronto para produção.

---

# 23. Conclusões técnicas sustentadas pelas evidências

## 23.1 O Cloud pode ser separado do NetSDK

A evidência suporta a separação:

```text
Cloud/T2U
```

e:

```text
Intelbras/NetSDK
```

O primeiro fornece o endpoint.

O segundo consome esse endpoint.

---

## 23.2 Não existe obrigação arquitetural de .NET

O fluxo real chega a:

```text
libt2u.dll
```

nativa.

Portanto, tecnicamente, uma futura implementação C++ pode trabalhar diretamente com a biblioteca nativa, desde que:

- ABI seja validada;
- contrato seja suportado;
- licença permita;
- bootstrap seja obtido de forma legítima.

---

## 23.3 O túnel deve ter lifetime por dispositivo/sessão

A evidência não suporta criar/destruir o túnel por tile.

O lifetime correto observado é maior que o lifetime de um RealPlay.

---

## 23.4 Estado de transporte precisa ser explícito

O sistema precisa distinguir:

- Direct P2P;
- Relay;
- desconectado;
- desconhecido.

`P2pIsActive` sozinho não fornece essa semântica.

---

## 23.5 Nenhum endpoint observado deve ser constante

Não hardcodear:

- IP de rendezvous;
- IP de Relay;
- porta TCP local;
- credential;
- endpoint observado em captura.

---

# 24. Restrições permanentes derivadas da investigação

Para a futura implementação SENTRIUM Cloud/P2P:

- não depender da interface ou processo do SIM Next;
- não aceitar “SIM Next instalado” como solução final do produto;
- não redistribuir DLLs sem autorização;
- não extrair/copiar credential secreta;
- não hardcodear infraestrutura observada;
- não declarar Direct sem prova;
- não usar `P2pIsActive` como sinônimo de Direct;
- não acoplar lifetime Cloud ao tile/RealPlay;
- não descarregar biblioteca nativa com callback/túnel ativo;
- não declarar integração pronta sem Windows limpo e matriz de compatibilidade.

---

# 25. Fontes internas utilizadas

Esta consolidação foi construída a partir de materiais já existentes no projeto e nas investigações anteriores:

1. `AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md` — revisão de 23/10/2026;
2. logs do **XC Video IA — Cloud Host/TFM Probe 003**;
3. logs do **XC Video IA — Exact Integration Probe 004 r2**;
4. registros do **Isolation Probe 002**;
5. README/CHANGELOG do XC Video IA v0.4/v0.4.2 — Cloud T2U;
6. documentação de contexto do SENTRIUM VMS;
7. observações históricas de tráfego, mantidas somente com o nível de confiança permitido pela auditoria.

---

# 26. Estado final deste documento

## Comprovado como baseline

- cadeia P2PSDK → Wrapper → libt2u;
- túnel T2U antes do NetSDK;
- listener TCP local;
- NetSDK TCP normal após túnel;
- Relay funcional;
- múltiplos probes em ~3,1 s;
- comportamento do `AddSDKPort`;
- lifetime independente do RealPlay;
- semântica Relay via `stat.proxy == 1`;
- `P2pIsActive` não equivale a Direct;
- disponibilidade de exports T2U principais.

## Ainda bloqueado / pendente

- Direct P2P real comprovado;
- bootstrap oficial;
- ABI final;
- redistribuição;
- standalone;
- Windows limpo;
- failover;
- compatibilidade atual;
- implementação C++ no SENTRIUM.

---

**Este documento deve ser tratado como referência factual da investigação Cloud/P2P e não como especificação final da API Intelbras.**

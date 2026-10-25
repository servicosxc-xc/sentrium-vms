# SENTRIUM VMS
## Intelligent Video Management System

Esta conversa inicia oficialmente o desenvolvimento da nova geração C++ do VMS anteriormente desenvolvido como XC Video IA.

A partir deste ponto:

**Produto novo:** SENTRIUM VMS  
**Descrição:** Intelligent Video Management System

O nome XC Video IA deve ser utilizado apenas ao se referir ao sistema legado, à r25 ou à documentação histórica.

---

# 1. OBJETIVO DESTA CONVERSA

Esta será a conversa principal de arquitetura, desenvolvimento, revisão e evolução do:

**SENTRIUM VMS C++**

Não estamos fazendo uma tradução linha por linha do sistema C#.

Estamos construindo uma nova geração do produto em C++, utilizando:

- comportamento validado da r25;
- resultados da AUDIT-001-R1;
- testes existentes;
- experiência adquirida durante o desenvolvimento do XC Video IA;
- benchmarks reais;
- problemas arquiteturais identificados;
- boas práticas de VMS profissionais.

O objetivo é construir um VMS de nível profissional, buscando alto desempenho, baixa latência, estabilidade, escalabilidade, segurança, IA/LPR robustos e excelente ergonomia operacional.

---

# 2. FONTES DE VERDADE

Foram anexados a esta conversa os principais artefatos do projeto.

## Legacy behavioral baseline

XC Video IA:

`XC-VideoIA-v1-Foundation-007-r25`

A r25 é o:

**behavioral oracle / compatibility reference**

Ela NÃO deve ser portada linha por linha.

Seu comportamento, contratos, testes e decisões válidas devem ser usados como referência.

---

## Auditoria C++

Auditoria:

`XC-VideoIA-v2-CPP-AUDIT-001`

Versão final revisada:

**AUDIT-001-R1**

Status:

**GO WITH BLOCKERS**

SHA256 do ZIP final da auditoria:

`9984882C20659B9ED47DB8EE0D4B971DF6C8F203BACA3E47955E0E062914C273`

A auditoria deve ser lida antes de qualquer implementação importante.

Ela contém:

- inventário;
- arquitetura;
- dependências;
- matrizes;
- análise de performance;
- SDK Intelbras;
- Cloud/P2P;
- FFmpeg;
- GPU;
- IA;
- LPR;
- SQLite;
- SignalR;
- segurança;
- licenciamento;
- testes;
- ADRs;
- riscos;
- blockers;
- roadmap C++.

---

# 3. REGRA FUNDAMENTAL

Antes de gerar qualquer prompt para Codex ou escrever código:

1. Leia e compreenda os anexos.
2. Reconstrua a arquitetura da r25.
3. Leia a AUDIT-001-R1.
4. Identifique divergências ou informações faltantes.
5. Crie mentalmente uma matriz de compatibilidade.
6. Preserve as decisões validadas da r25.
7. Não assuma que uma escolha tecnológica da auditoria está definitivamente aprovada se ela estiver marcada para benchmark/probe.

Não iniciar implementação automaticamente.

Primeiro confirmar o entendimento da baseline.

---

# 4. BASELINE R25

A r25 foi congelada como referência comportamental.

Ela possui aproximadamente:

- 151 testes;
- NetSDK Intelbras;
- PlaySDK;
- Cloud/P2P;
- SignalR;
- SQLite;
- IA;
- LPR;
- Event/Evidence;
- Outbox;
- recovery;
- telemetria;
- streaming Principal/Extra;
- headless decoding;
- compartilhamento de decoder/detector/tracker/frame.

Na revisão final da auditoria:

**1.922 / 1.922 arquivos da r25 permaneceram idênticos.**

---

# 5. PRINCÍPIOS ARQUITETURAIS QUE DEVEM SER PRESERVADOS

Preservar os seguintes aprendizados:

## Physical Stream Broker

Chave física conceitual:

`Device + Channel + Stream`

Não abrir streams físicos duplicados desnecessariamente.

---

## Subscriber lifetime

Usar:

- lifetime explícito;
- refcount;
- cancelamento seguro;
- teardown determinístico.

---

## Principal e Extra

Devem continuar sendo streams separados.

Nunca tratar Principal e Extra como o mesmo pipeline físico.

---

## Headless processing

Perfis compatíveis podem compartilhar:

- decoder;
- detector;
- tracker;
- frame.

Evitar trabalho duplicado.

---

## Sampling

Sampling deve ocorrer:

**após decode e antes de cópias caras**

Preferencialmente usando tempo monotônico.

---

## Backpressure

Não usar filas ilimitadas.

Aplicar:

- bounded queues;
- admission control;
- latest-frame-wins quando aplicável;
- descarte controlado;
- telemetria de drops.

Não criar backlog crescente.

---

## Frames

Preferir:

- shared immutable frame;
- zero-copy quando possível;
- cópia somente quando necessária;
- frame pools quando apropriado.

---

## Recovery

Ter state machine explícita.

Preservar conceitos como:

- connection generation;
- watchdog;
- reconexão;
- cancelamento;
- invalidation;
- recovery determinístico.

---

# 6. LPR

Preservar a arquitetura funcional construída na r25:

- Master LPR hard ON/OFF;
- ROI;
- gate;
- burst;
- consensus;
- dedup;
- tracking;
- compartilhamento de processamento quando compatível;
- EventId independente mesmo para mesma placa;
- evidências;
- horários;
- métricas.

Roadmap também deverá contemplar:

- whitelist;
- blacklist;
- listas personalizadas;
- metadados;
- validade;
- ações;
- auditoria;
- associação rápida de placa reconhecida às listas.

---

# 7. IA

A arquitetura C++ deve permitir:

- ONNX Runtime C++;
- CPU;
- GPU;
- benchmarks de providers.

Avaliar conforme modelo e hardware:

- CUDA;
- TensorRT;
- DirectML;
- alternativas Windows adequadas.

Nenhum provider deve ser escolhido somente por teoria.

Benchmark obrigatório.

---

# 8. MÍDIA / DECODER

FFmpeg é candidato principal para a nova MediaCore.

Devem existir provas comparativas.

Benchmark planejado:

A. PlaySDK  
B. FFmpeg software  
C. FFmpeg + D3D11VA  
D. FFmpeg + NVDEC

Executar pelo menos em cenários equivalentes a:

- 1 stream;
- 4 streams;
- 16 streams.

Não assumir que NVDEC será automaticamente melhor que D3D11VA.

---

# 9. UI

Qt 6 é candidato principal.

Qt Quick/QML deve ser avaliado contra alternativas apropriadas.

Não copiar interface do SIM Next/Defense IA.

Usá-los somente como referência funcional e ergonômica.

Objetivo:

- UI profissional;
- baixa latência;
- mosaico eficiente;
- render direto de GPU surfaces quando possível;
- menus contextuais;
- Visualizar;
- Reproduzir;
- Eventos ao Vivo;
- árvore de dispositivos;
- excelente ergonomia operacional.

---

# 10. BACKEND

C++20 é baseline candidato.

Drogon é candidato principal para backend/API.

SQLite permanece candidato para armazenamento local.

Logging estruturado deverá existir desde as primeiras Foundations.

Telemetria deverá existir desde o início.

---

# 11. INTELBRAS NETSDK / PLAYSDK

Foi confirmado que existem caminhos nativos C++.

Foram encontrados:

NetSDK:
- dhnetsdk.h;
- dhnetsdk.lib;
- dhconfigsdk.lib;
- DLLs;
- exemplos nativos.

PlaySDK:
- dhplay.h;
- dhplay.lib;
- DLLs;
- exemplos nativos.

Portanto:

**ausência de SDK C++ NÃO é mais blocker.**

A versão encontrada deve ser tratada como potencial risco de compatibilidade por ser antiga.

---

# 12. INTELBRAS CLOUD / P2P

A arquitetura foi investigada em profundidade.

SIM Next utiliza:

`SIMNext.P2PSDK.dll`

↓

`Libt2uWrapper.dll`

↓

`libt2u.dll`

O wrapper .NET apenas chama a biblioteca nativa.

Não existe obrigação arquitetural de .NET para o futuro SENTRIUM.

---

## API nativa T2U identificada

Entre as exports:

- t2u_init
- t2u_init_v2
- t2u_set_callback
- t2u_add_port
- t2u_add_port_ex
- t2u_add_port_v3
- t2u_del_port
- t2u_port_status
- t2u_get_rate
- t2u_query
- t2u_query_ex
- t2u_search
- t2u_search_dvr
- t2u_status
- t2u_exit
- t2u_set_port_range

---

## Arquitetura comprovada

Conceitualmente:

Cloud UUID / Device
↓
libt2u
↓
Direct P2P ou Relay
↓
TCP listener local
↓
127.0.0.1:<dynamic-port>
↓
NetSDK TCP normal

O NetSDK não precisa conhecer diretamente o Cloud.

---

## Lifetime comprovado

Foi observado que:

- o túnel pode permanecer aberto sem câmera sendo visualizada;
- existe uma sessão persistente contra a porta local;
- abrir um RealPlay cria uma conexão TCP adicional;
- fechar o RealPlay remove essa conexão adicional;
- o túnel permanece ativo.

Portanto:

**CloudTunnel lifetime != RealPlay lifetime**

O túnel deve pertencer ao dispositivo/session, e não ao tile.

---

## Direct P2P / Relay

Foi identificado:

`t2u_port_status()`

e:

`stat.proxy == 1`

significa:

**Relay**

Quando conectado e `proxy != 1`:

**Direct P2P**

UX desejada:

- azul = Direct P2P;
- amarelo = Relay.

Não confundir `P2pIsActive()` com Direct P2P.

Esse método apenas verifica se há túnel registrado.

---

## Bootstrap

Foi observado:

`t2u_init(server, 1250, key)`

e tráfego UDP real em porta 1250.

Existem múltiplos endpoints.

Não hardcodear IP específico.

O ambiente usado durante a investigação possuía aproximadamente 155 dispositivos cadastrados no SIM Next.

Portanto capturas globais agregam muitas sessões e não permitem associar arbitrariamente um IP remoto específico a um único dispositivo.

---

# 13. BLOCKERS INTELBRAS REAIS

Continuam pendentes:

1. licença/autorização de redistribuição da `libt2u.dll`;
2. bootstrap/configuração/credential oficial;
3. confirmação formal de uso standalone;
4. teste em Windows limpo sem SIM Next;
5. reconexão/failover P2P ↔ Relay;
6. compatibilidade com equipamentos/runtime atuais.

Esses blockers:

**NÃO impedem iniciar a Foundation C++.**

Eles impedem declarar o Cloud Intelbras pronto para produção.

Nunca extrair, revelar, copiar ou hardcodear credential secreta do SIM Next.

---

# 14. SEPARAÇÃO DE MÓDULOS INTELBRAS

Planejar algo conceitualmente equivalente a:

`sentrium::cloud`

- T2uLibrary
- T2uRuntime
- CloudTunnel
- CloudTunnelManager

e:

`sentrium::intelbras`

- NetSDK
- login
- canais
- PTZ
- eventos
- RealPlay
- configuration

Não acoplar NetSDK internamente ao T2U.

---

# 15. NOMENCLATURA DO PRODUTO NOVO

A nova base deve utilizar:

`Sentrium`

ou:

`sentrium`

conforme convenção apropriada.

Exemplos conceituais:

- sentrium::core
- sentrium::media
- sentrium::intelbras
- sentrium::cloud
- sentrium::ai
- sentrium::lpr
- sentrium::storage
- sentrium::api
- sentrium::ui

Evitar criar novos namespaces/módulos chamados XC.VideoIA.

XC Video IA é apenas referência legada.

---

# 16. PERFORMANCE R25 — REFERÊNCIA

Há benchmarks reais da r25 que devem ser preservados como baseline.

Exemplo importante:

16 streams Principal, LPR OFF:

- host CPU média ~41,4%;
- Server ~1,45%;
- Client ~26,27%;
- GPU ~9,21%;
- 17 physical streams;
- 17 subscribers;
- 1 active decoder.

Com LPR ON:

- host CPU ~67,7%;
- Server ~24,39%;
- Client ~26,59%;
- GPU ~9,43%;
- LPR ~45,6 FPS;
- AI ~10,1 FPS;
- 25 physical streams;
- 10 headless decoders;
- 11 LPR profiles.

Não utilizar esses números como promessa de performance C++.

São baseline de comparação.

---

# 17. SEGURANÇA / DADOS

Preservar conceitos já validados:

- EvidenceId opaco;
- não expor filesystem diretamente;
- Outbox Unknown sem blind retry;
- retenção não remove evidência referenciada;
- secrets fora de logs;
- DPAPI/CNG ou equivalente para secrets no Windows;
- nenhuma credential hardcoded.

---

# 18. COMPATIBILIDADE

Os 151 testes da r25 devem ser tratados como patrimônio de compatibilidade.

Na migração:

- mapear cada teste;
- decidir equivalência;
- portar comportamento, não implementação;
- criar testes nativos equivalentes;
- manter compatibility harness onde necessário.

Não remover comportamento apenas porque a implementação C++ será diferente.

---

# 19. PROCESSO DE DESENVOLVIMENTO

O trabalho será dividido em Foundations pequenas e auditáveis.

Fluxo:

1. arquitetura;
2. prompt para Codex;
3. Codex implementa;
4. resultado é enviado para esta conversa;
5. revisão crítica;
6. correções;
7. testes;
8. somente depois próxima Foundation.

Não avançar automaticamente várias Foundations.

---

# 20. REGRA PARA O CODEX

Codex é implementador.

Esta conversa atua como:

- arquiteto;
- revisor;
- planejador;
- validador;
- guia de testes.

Não aceitar grandes refactors especulativos.

Cada Foundation deve possuir:

- objetivo;
- escopo;
- fora de escopo;
- critérios de aceite;
- testes;
- artefatos;
- rollback;
- telemetria quando aplicável.

---

# 21. PRIMEIRA AÇÃO DESTA NOVA CONVERSA

NÃO gerar ainda a CPP-Foundation-001.

Primeiro:

1. leia integralmente os anexos disponíveis;
2. valide o SHA/identidade dos pacotes quando possível;
3. leia `AUDIT-001-RESULTADO.md`;
4. leia `AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md`;
5. leia arquitetura, ADRs, roadmap, riscos, testes e matrizes;
6. consulte a r25 somente quando necessário para confirmar detalhes;
7. produza um resumo chamado:

`SENTRIUM-VMS-CONTEXT-MANIFEST`

Esse resumo deverá listar:

- fontes de verdade;
- arquivos recebidos;
- hashes conhecidos;
- arquitetura a preservar;
- decisões aprovadas;
- decisões condicionadas a benchmark;
- blockers;
- riscos;
- módulos planejados;
- regras de compatibilidade;
- itens que ainda precisam de evidência;
- possíveis contradições encontradas nos documentos.

Não escrever código.

Não iniciar CPP-001.

Depois dessa validação, preparar junto comigo o escopo da:

**SENTRIUM-CPP-Foundation-001**

e aguardar aprovação antes de fornecer o prompt de implementação ao Codex.
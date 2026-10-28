# F005 — Direcao sem SDK Cloud do fornecedor: desenvolvimento executavel

Data: 28/10/2026. Decisao contextual: operador informa que o unico canal oficial disponivel a ele e um chatbot, que oferece apenas SDK DVR/NVR NetSDK convencional (kit ja entregue ao projeto). **Nao depender de atendimento humano ou fornecimento futuro de SDK Cloud Intelbras para manter a F005 avancando.** Nao significa autorizacao implicita para uso de credenciais privadas dos servicos Intelbras.

## Objetivo original preservado e limite de equivalencia

O objetivo original permanece: SENTRIUM VMS C++/Qt independente, que consiga conectar dispositivos Intelbras remotos sem instalar SIM Next e, quando possivel, sem redirecionar portas no cliente. A conexao **exata** pelo ID Cloud/serial de fabrica usando os servidores Intelbras ainda NAO FOI COMPROVADA para app de terceiro. RE-002/003 provaram stream UDP em sessao do **SIM Next licenciado** com endpoint WAN real da rede do DVR; NAO provaram cadastro/autenticacao independente do SENTRIUM nos servidores Intelbras. A mera implementacao de um envelope 12 01 10 xx nao concede autenticacao Cloud.

## Estrategia pratica em paralelo — sem bloquear o projeto

### Linha A — Registro Automatico NetSDK ja existente (PRIORIDADE de produto)

- Fundamentacao: o NetSDK oficial ja esta presente no projeto. O usuario anteriormente demonstrou **um NVR real de 8 canais conectado por AutoRegistration**. Codigo atual de producao usa `CLIENT_ListenServer`, callbacks de auto registro e `CLIENT_LoginEx2(... EM_LOGIN_SPEC_CAP_SERVER_CONN ...)` com ID configurado no gravador, NAO serial Cloud. Regressao e resultados de laboratorio continuam limitados ao caso verificado.
- O gravador inicia conexao de saida ao servidor de Registro Automatico, o que pode funcionar atras de CGNAT no **lado do gravador** sem abrir portas no seu roteador, desde que alcance um endereco/porta do servidor. Um listener privado no PC do operador sozinho NAO fica automaticamente publico; para clientes remotos e necessario endpoint central alcançavel (servidor com IPv4 publico e firewall especifico) ou VPN/site-to-site.
- Configuracao remota e consentimento sao por instalacao; o ID de Registro e administrado pelo operador e NAO substitui o numero de serie Cloud Intelbras. O lado do servidor deve ter autenticação/segregacao, controle de clients esperados e trilha de auditoria. Nao presumir que protocolo nativo de registro e criptografado; em redes nao confiaveis, proteger o transporte usando VPN/WireGuard ou outro canal revisado antes de producao.
- Proxima engenharia: revisar estado do listener, limitar bind a interface apropriada, controles de reconexao e timeouts/stale sessions, UI separando `Configurações > Registro Automático` de `Adicionar dispositivo`, diagnostico de registro e enumeracao real. Fazer testes focados em laboratorio e novo teste de rede remota consentido antes de prometer acesso via Internet.

### Linha B — Acesso IP privado por VPN/tunel (fallback profissional)

- Para redes com roteador administravel (ex.: MikroTik), WireGuard site-to-site com endpoint controlado permite o SENTRIUM usar o login NetSDK IP tradicional sobre rede privada. Pode atravessar CGNAT no lado do cliente com conexao originada do local e servidor VPN alcançavel; requer equipamento/router/gateway que mantenha o tunel, ACLs e suporte.
- Alternativa operacional e gateway gerenciado, instalado com permissao no local do cliente. **Nao** depende de credenciais nem servicos P2P da Intelbras, mas nao e equivalente a inserir o serial e obter video sem configuracao.

### Linha C — pesquisa de interoperabilidade Cloud P2P Intelbras (SEM SDK, NAO PRODUTIVO)

- Preservar docs RE-001..RE-004 como evidencias de compatibilidade a estudar. Headers 12 20 D0/D1/D2 e 12 01 10 xx, servidor DNS/HTTPS e UDP transporte sao fenomenos observados; sem inferir identidade app, chave, autenticacao, licenca, direta/rele ou NetSDK tunnel.
- Estudos futuros limitados a analise passiva, dados anonimizados, comportamento autorizado e referencias de especificacoes legalmente utilizaveis. Nao reutilizar DLLs SIM Next, credenciais de app, endpoints internos nao autorizados, tokens de terceiro ou contornar protecoes. Sem autenticação legitima, a via por serial fica estado **BLOCKED**, sem conexao falsa nem cor falsa de Direct/Relay.
- Fonte oficial historica e gateway de licenciamento passam a **DEPENDENCIA EXTERNA NAO ACESSIVEL ATUALMENTE**, sem repeticao de contato por chatbot; podem ser reavaliados somente se surgir caminho legitimo.

## Priorizacao executavel

| Trilha | Estado atual | Proximo marco verificavel |
|---|---|---|
| LAN/NetSDK | Funcional em casos medidos | Preservar como baseline |
| AutoRegistro/NetSDK | NVR 8ch testado em laboratorio | Prova em rede remota consentida com servidor seguro; reconexao e arvore reais |
| IP por VPN | Alternativa de arquitetura, nao testada na F005 | Laboratorio WireGuard com rede privada e login NetSDK |
| Cloud Intelbras por serial | Pesquisa passiva de protocolo, SEM login independente | Evidencia legitima de autorizacao/bootstrap, caso disponivel; nao bloquear trabalho acima |

## Governanca

Esta decisao **nao** reinterpreta F005-AC-03/04 como PASS, nao faz merge e nao conclui Human Review. Registros locais, segredos e PCAPs nao entram no GitHub. Nenhum listener publica automaticamente portas, nenhuma alteracao de rede ou firewall de cliente sem autorizacao especifica. O titulo da trilha Cloud permaneceu na F005 por historico, sem promessas de interoperabilidade P2P nativa. O plano deve ser executado incrementalmente na branch foundation/005-device-connectivity com build/test Windows comprovados a cada mudanca de codigo.

Referencias oficiais: manual SIM Next https://manuais.intelbras.com.br/manual-sim-next/simnext.html (modos IP/Dominio/Cloud P2P/Relay/Registro Automatico); manual NVR Intelbras https://backend.intelbras.com/sites/default/files/2026-03/Manual_NVD_1516P_01-26_site_0.pdf (AutoRegistro e identificador configurado); docs Tailscale https://tailscale.com/docs/features/subnet-routers para alternativa rede privada.

## Excecao controlada — feasibility LAB-001 com DLLs originais (28/10/2026)

A pedido explicito do operador, foi criado experimento de desenvolvimento separado do produto para verificar integridade, runtime e possivel criacao de tunel com bibliotecas **existentes** no diretorio de instalacao local SIM Next. NAO faz copia, redistribuicao, implantacao comercial ou integrado ao C++ no momento. Qualquer CONNECT precisa confirmacao expressa e direitos aplicaveis; a via por serial no SENTRIUM permanece BLOCKED para produto ate prova integral de login, seguranca e licenca. Detalhes docs/foundation-005/F005-SIMNEXT-CLOUD-LIB-LAB-001.md. As alternativas AutoRegistro e VPN permanecem validas.

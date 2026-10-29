# F005 LAB-001 — isolamento para o listener Cloud T2U e proximo gate NetSDK

**Data:** 28/10/2026. **Estado:** DESIGN / NAO APLICADO. Nenhuma regra do Firewall foi criada, nenhuma credencial NetSDK passou pelo listener. Nao confundir desenho de isolamento com autorizacao de operacao comercial das DLLs.

## Evidencia real consolidada

O operador executou em Visual Studio 2022 Developer PowerShell Windows o laboratorio C# standalone, carregando cinco DLLs **do diretorio local do SIM Next**. A ultima execucao foi consistente e concluiu:

- `CLOUD_ID_INPUT: MATCH`: duas entradas privadas identicas (valor sensivel NAO retido no repositorio).
- `ADAPTER_READY`: tipo e metodos encontrados e objeto instanciado.
- `ADD_PORT: elapsed_ms=1921; returned=VALID_PORT`: `AddSDKPort` retornou porta TCP local valida.
- `TUNNEL STATE: ACTIVE`: `P2pIsActive` retornou true, observacao de estado do fornecedor **nao** prova identidade/autenticacao fim-a-fim.
- `TUNNEL ROUTE: NOT_RELAY_REPORTED`: **NAO PROVA DIRECT P2P** nem rota autenticada.
- `LOCAL TCP LISTENER: OBSERVED`; `BIND CATEGORIES (no IP): WILDCARD_IPV4`: escuta em `0.0.0.0`, ou seja, interfaces IPv4 locais.
- `LISTENER OWNER: CURRENT_LAB_PROCESS`: Windows IP Helper atribuiu a porta ao PID do **harness de laboratorio**, nao a processo de SIM Next.
- `SAFETY STOP` e exit9: devido a bind nao restrito ao loopback; a DLL encerrada com `DeleteSDKPort` e `Cleanup` tentados em finally.

**Interpretacao limitada:** a instalacao local fornece bibliotecas que, num processo criado para o SENTRIUM e sem a interface de SIM Next em execucao, reportam a abertura de um tunel Cloud com listener TCP. Nao equivale a biblioteca redistribuivel, nao prova Windows limpo, atraves de NetSDK com login/video nem que nenhum processo auxiliar seja necessario. A porta escuta em `0.0.0.0`, mas firewall/NAT podem ou nao permitir acesso de terceiros. Portanto NAO enviar credenciais atraves dela sem isolamento testado.

## Por que nao basta um proxy em 127.0.0.1?

Um proxy local na frente do listener NAO modifica o socket original em `0.0.0.0`; possiveis conexoes de fora ainda poderiam atingir a porta T2U original. Uma protecao precisa afetar o listener original ou a exposicao de rede. Como os binarios sao proprietarios e o processo nativo foi identificado, a estrategia nao-invasiva preferida e bloquear **entrada TCP ao executavel de laboratorio** por politica de Windows Firewall e comprovar o resultado em equipamento de teste, mantendo a **saida de rede** necessaria ao P2P.

## Plano de isolamento controlado (EXECUCAO PENDENTE DE APROVACAO EXPLICITA)

1. Garantir que **Windows Firewall esteja ativo nos perfis relevantes**, que regras locais sejam aplicadas no contexto do Windows e que nao haja restricao por politicas empresariais que tornem uma regra local ineficaz. Registrar so categorias/status, sem IPs pessoais.
2. Compilar antecipadamente `tools/simnext-cloud-lab/Sentrium.CloudLibLab.csproj` com `dotnet build -c Release`. Determinar o caminho completo de `tools/simnext-cloud-lab/bin/Release/net10.0/Sentrium.CloudLibLab.exe`. Nao copiar nenhuma DLL Intelbras. **Nao executar as DLLs proprietarias como Administrador.**
3. Em terminal administrador SEPARADO, com consentimento do operador, criar **uma unica regra temporaria de BLOQUEIO de entrada**, exclusivamente para este `Sentrium.CloudLibLab.exe`, protocolo TCP, todos os perfis, com nome exclusivo auditavel. **Nao alterar regras de outros programas, nao abrir portas, nao desligar Defender**. Priorizar regra por programa pois porta local e dinamica. Exemplo de configuracao a ser automatizada sob gate: `New-NetFirewallRule -Name <LAB-UNIQUE> -Direction Inbound -Program <LAB_EXE_ABSOLUTO> -Action Block -Protocol TCP -Profile Any -Enabled True`. Validar a regra efetivamente cadastrada, programa associado e politica. Esta sintaxe e documentada pela Microsoft: https://learn.microsoft.com/en-us/powershell/module/netsecurity/new-netfirewallrule
4. Executar o **laboratorio** numa conta/processo **nao elevados** apos regra ativa. Se `AddSDKPort` fornecer porta, confirmar novamente ownership/bind WILDCARD, mantendo como fato o bind original (nao disfarcar como `LOOPBACK_ONLY`).
5. Com o laboratorio aberto, realizar verificaçoes separadas **sem credenciais**: (a) conexao TCP a `127.0.0.1:<porta dinamica>` do proprio PC; (b) tentativa de conexao TCP a essa mesma porta a partir de outro host autorizado da mesma LAN. A exigencia: localhost funciona e host LAN nao consegue conectar. `Test-NetConnection` de localhost SOZINHO nao comprova bloqueio na LAN. Se nao houver segunda maquina na rede, registrar **LAN BLOCK NOT RUN** e NAO prosseguir para NetSDK.
6. Apenas com verificacao positiva de isolamento e permissao de avaliacao das DLLs: ensaio de login NetSDK COM CONTAS DE TESTE e sem trafego de credenciais nos logs, seguido de enumeracao de canais reais. Validar logout e teardown; nao afirmar Direct ate haver evidencias concretas de rota.
7. Remover SOMENTE a regra temporaria do laboratorio por nome exclusivo ao finalizar, seja sucesso ou erro (em script automatizado, `finally` e comando de recuperacao separado). A falta de rollback nao deve resultar em firewall aberto, mas uma regra residual de block pode precisar limpeza. Conferir ausencia da regra e nenhum processo/listener remanescente.

**Importante:** o operador **ainda nao autorizou** qualquer alteracao no firewall. Nenhum script deve criar/remover regras neste checkpoint. Nao alterar firewall remotamente via GitHub ou ferramentas de desenvolvimento. Nao contornar licenca ou pegar credenciais de inicializacao do SIM Next. Se o sistema estiver sob politicas que bloqueiam regras locais ou nao for possivel comprovar inacessibilidade LAN, abortar.

## Arquitetura seguinte

Manter a biblioteca de laboratorio separada do C++ SENTRIUM VMS. Depois de comprovar controle NetSDK num ambiente seguro, projetar um adaptador com ciclo de vida: `CloudDeviceSession` → provedora T2U apenas no LAB com direito de uso → endpoint dinamico protegido → NetSDK → login/channels → teardown. O produto definitivo depende de direito de uso/distribuicao dos componentes, teste Windows limpo e suporte de bootstrap sem depender da instalacao de SIM Next. **F005 Cloud product** permanece `BLOCKED`, enquanto o `LAB-001` e `PARTIAL CONNECTIVITY OBSERVED`.

**Controle de qualidade:** ultimo C++ R10 Windows D-41..D-46 6/6 PASS em tests sinteticos, **nao** login Cloud. PR #4 draft, `main` e F004 Human Review intactos.

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


## Implementacao apos aprovacao do operador — firewall + modo IsolationProbe (28/10/2026)

O usuario confirmou expressamente a criacao de uma regra **temporaria, especifica do executavel de laboratorio e exclusivamente de BLOQUEIO TCP de entrada**. Isso NAO autoriza desabilitar firewall, abrir portas, mudar regras de outros aplicativos ou executar o VMS como Administrador.

Adicionado ao repositorio:

- `scripts/windows/sentrium-cloud-lab-firewall.ps1`: `-Action Prepare` compila EXE apphost .NET10 x64 no perfil do usuario **nao elevado**, pede UAC em processo separado SOMENTE para `-Action Install`, cria `SENTRIUM-F005-LAB-TCP-IN-BLOCK` (`Inbound`, `TCP`, `Block`, `Profile Any`, programa absoluto `Sentrium.CloudLibLab.exe`, portas `Any`), confirma perfil ativo, politica de regras locais, `PersistentStore` e `ActiveStore`. Falha se regras nao se aplicarem. `-Action Status` e somente leitura; `-Action Remove` pede UAC e remove APENAS a regra propria, nunca mexe em firewall global.
- `scripts/windows/sentrium-simnext-cloud-lab.ps1 -Mode IsolationProbe`: executa um teste SEM senha ou login NetSDK, somente quando `-Action Status` aprova e em sessao **nao elevada**; chama diretamente o `.exe` gerado (evita `dotnet.exe` como processo host inesperado). Ao sair tenta UAC de `-Action Remove`; se UAC for recusado a regra de BLOCK podera permanecer e devera ser removida pelo comando de recuperacao.
- `tools/simnext-cloud-lab/Program.cs` modo `isolation-probe`: exige `CURRENT_LAB_PROCESS`, abre somente TCP em `127.0.0.1` sem credencial e reporta `ISOLATION LOOPBACK TCP`; caso conectado, imprime porta TCP dinamica e aguarda ENTER com o tunel vivo. O uso de DLLs do SIM Next permanece restrito a pasta local do operador, sem copia ou commit.

### Execucao guiada: primeiro passo apenas

No **Developer PowerShell normal, NAO Administrador**, executar:

    cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"; git pull --ff-only origin foundation/005-device-connectivity; if ($LASTEXITCODE -eq 0) { .\scripts\windows\sentrium-cloud-lab-firewall.ps1 -Action Prepare }

Deve ocorrer: `dotnet build ... Release` e janela de UAC so para regra de Firewall, depois `F005 FIREWALL STATUS PASS` e `F005 PROTECT READY`. Neste passo **nenhuma DLL Cloud e executada nem ha rede Cloud**.

Segundo passo SOMENTE apos receber log positivo do primeiro: `scripts/windows/sentrium-simnext-cloud-lab.ps1 -Mode IsolationProbe`; selecionar DLL da instalacao original, confirmar, digitar identificador duas vezes SEM eco e porta de servico localmente. Com `ISOLATION PORT`, o teste precisa ser feito a partir de **outro PC da LAN**: `Test-NetConnection <IP-LAN-DO-PC-DO-LAB> -Port <PORTA_TEMPORARIA> -InformationLevel Quiet` deve dar `False`; localhost deve estar `CONNECTED`. Se o segundo PC nao estiver disponivel, LAN e `NOT RUN`, nao ha liberacao para credenciais. Nao enviar IP, serial ou senha ao chat.

Pressionar ENTER no laboratorio fecha o tunel e o wrapper solicita um segundo UAC para remover a regra. Verificar `F005 FIREWALL REMOVED`. Em caso de cancelamento ou falha no rollback, depois de encerrar o LAB rodar `.\scripts\windows\sentrium-cloud-lab-firewall.ps1 -Action Remove` na Developer PowerShell normal e aceitar UAC.

**Gate:** somente apos bloqueio externo realmente confirmado, conta de teste autorizada e licenca aplicavel consideraremos um ensaio NetSDK separado. Regra ativa nao basta para provar firewall. Nenhuma credencial NetSDK e solicitada nesse fluxo. Os novos codigos ainda NAO foram compilados/testados no Windows; nao antecipar PASS. Produto C++/main/F004 e PR draft permanecem intactos.

Refs Microsoft: https://learn.microsoft.com/en-us/powershell/module/netsecurity/new-netfirewallrule ; https://learn.microsoft.com/en-us/powershell/module/netsecurity/get-netfirewallapplicationfilter ; https://learn.microsoft.com/en-us/powershell/module/netsecurity/remove-netfirewallrule ; https://learn.microsoft.com/en-us/powershell/module/netsecurity/get-netfirewallprofile .


## Ajuste de compatibilidade de permissao — 28/10/2026

O operador tentou `-Action Prepare` na Developer PowerShell e depois num PowerShell aberto normalmente por Win+R; **AMBOS** foram reconhecidos como `Is-Admin=True` pelo Windows. O script anterior interrompeu antes de compilar, instalar regra ou carregar DLL Cloud. Isso nao demonstra configuracao UAC especifica; exige checagem posterior se o usuario precisa de token limitado para o passo de conexao.

Foi ajustado apenas `scripts/windows/sentrium-cloud-lab-firewall.ps1` para permitir o `Prepare` em console elevado e nao elevado. Ele registra categoria `F005 PREPARE TOKEN: ELEVATED|STANDARD`. Em console elevado, executa a compilacao .NET e invoca `Install` da mesma regra BLOCK diretamente, sem outro prompt UAC; em console padrao, continua solicitando UAC para `Install`. `Install` continua validando nome, filtro do executavel, firewall por perfil, integridade da regra e estado ativo. **Nenhuma DLL SIM Next e executada no Prepare**, nenhum login NetSDK, nenhum listener Cloud ou captura. 

O modo `-Mode IsolationProbe` continua exigindo token NAO elevado e um segundo host LAN para validar bloqueio antes de usuario/senha NetSDK. A permissao para criar regra NAO e permissao para executar a DLL proprietaria como admin. Se os shells comuns continuarem aparecendo elevados, diagnosticar conta/UAC ou usar uma conta de laboratorio padrao; **nao remover o gate do IsolationProbe**. **Nova versao ainda nao executada no Windows.** `-Action Remove` continua disponivel caso alguma regra precise ser revertida apos um Prepare bem-sucedido.


## PREPARE Windows PASS — token elevado, BLOCK instalada (28/10/2026)

O operador executou `git pull` ate `894c1823` e `-Action Prepare`. Evidencia real: `F005 PREPARE TOKEN: ELEVATED`, `dotnet build Release net10.0 êxito`, criacao da regra `SENTRIUM-F005-LAB-TCP-IN-BLOCK` e verificacoes `F005 FIREWALL STATUS PASS` (duas vezes), `F005 PROTECT READY`. Nenhuma biblioteca proprietaria foi executada. Regra permanece instalada no Windows do operador ate que ele chame `Remove` ou wrapper `IsolationProbe` encerre e solicite a reversao. Nao declarar isolamento LAN sem teste remoto.

O usuario ja havia iniciado **duas** shells PowerShell que receberam token elevado inclusive via Win+R. Em vez de executar bibliotecas proprietarias como admin ou desativar UAC, foi adicionado modo `-Action DiagnoseToken` **read-only** no script Firewall. Ele imprime apenas `CURRENT_TOKEN` (ELEVATED/STANDARD), `UAC` (ENABLED/DISABLED/UNKNOWN), `BUILTIN_ADMIN_ACCOUNT` (YES/NO) e `BUILTIN_ADMIN_APPROVAL_MODE` (ENABLED/DISABLED/UNKNOWN), sem SID/nome/IP. Dessa forma, podemos distinguir UAC desabilitado, conta Administrator interna sem filtro ou contexto elevado inesperado e orientar a abertura de sessao padrao. Esse comando NAO altera firewall e NAO executa DLL Cloud. Quando obtivermos sessao STANDARD, poderemos rodar `-Mode IsolationProbe` ainda com protecao instalada; login NetSDK e video permanecem NOT RUN.


## Reparacao critica de sintaxe — firewall DiagnoseToken (29/10/2026)

O operador executou `-Action DiagnoseToken` na branch `2b41fbd4` e recebeu `ParserError` antes de qualquer acao, com linha 83 `Token 'F005' inesperado` e final com parenteses invalido. A auditoria do texto GitHub constatou **corrupcao do arquivo no proprio repositorio**, nao causada por fechamento do PowerShell: durante a adicao de DiagnoseToken a expressao `$builtinAdmin = (... -match '-500$')` foi partida no literal `'-500` e uma COPIA INDEVIDA de todo o switch Prepare/Install/Status/Remove foi inserida dentro da funcao. Havia duplicacao de blocos e `)` solto. Correcao cirurgica removeu **somente a regiao duplicada** e restaurou a regex completa da conta Administrator, preservando o switch final e os controles do firewall. O patch foi inspecionado com contagem de ocorrencias (`switch($Action)` uma vez e cada caso uma vez), mas **parser PowerShell real ainda precisa executar no PC do operador**. Proximo teste deve invocar `[System.Management.Automation.Language.Parser]::ParseFile(...)` antes do script, rejeitando qualquer erro de sintaxe e **sem executar nenhuma acao de firewall**; em seguida rodar `-Action DiagnoseToken` read-only.

O `Prepare` anterior foi PASS em Windows e instalou a regra BLOCK dedicada ao LAB; um ParseError no script atualizado **nao removeu** essa regra nem executou o programa Cloud. `IsolationProbe` e NetSDK login permanecem pendentes. `main`, C++ e F004 Human Review continuam intocados. Nao pedir Prepare outra vez.


## Windows DiagnoseToken REAL PASS — UAC efetivamente desligado (29/10/2026)

**Evidencia do operador, apos pull de fc409df6 e validacao nativa do Parser PowerShell:** `PARSER PASS`, `TOKEN DIAG: CURRENT_TOKEN=ELEVATED`, `TOKEN DIAG: UAC=DISABLED`, `TOKEN DIAG: BUILTIN_ADMIN_ACCOUNT=NO`, `TOKEN DIAG: BUILTIN_ADMIN_APPROVAL_MODE=DISABLED`. O erro de sintaxe anterior foi corrigido. O valor `BUILTIN_ADMIN_APPROVAL_MODE=DISABLED` refere-se a conta interna Administrator, **que o diagnostico diz NAO estar em uso**; o fato operacional determinante e `UAC=DISABLED` para um usuario membro do grupo de administradores, permitindo a execucao elevada padrao. Nao significa que a conta do usuario seja o Administrator interno.

A documentacao oficial da Microsoft informa que `EnableLUA=0` desativa Admin Approval Mode e os tokens filtrados de administrador; em tais sessoes, processos podem receber privilegios completos sem prompt. **Nao habilitar UAC automaticamente**, pois e alteracao global de sistema e requer reinicializacao para a politica aplicar. Referencias: https://learn.microsoft.com/pt-br/windows/security/application-security/application-control/user-account-control/settings-and-configuration ; https://learn.microsoft.com/en-us/windows/client-management/mdm/policy-csp-localpoliciessecurityoptions .

**Caminho recomendado, controlado pelo operador:** verificar se ha aplicativos legados que dependem de UAC desligado; manualmente abrir `UserAccountControlSettings` (Painel de Controle > Sistema e Seguranca > Alterar configuracoes de Controle de Conta de Usuario), selecionar o nivel padrao de avisos acima de `Nunca notificar`, confirmar e reiniciar o Windows conforme exigencia da politica `EnableLUA`. Depois abrir PowerShell pelo menu Iniciar SEM `Executar como Administrador`; rodar `scripts/windows/sentrium-cloud-lab-firewall.ps1 -Action DiagnoseToken`. Gate exigido para o proximo passo: `UAC=ENABLED` **e** `CURRENT_TOKEN=STANDARD`. Se nao puder modificar politica global, alternativa e conta de teste local **padrao, nao membro de Administradores**, com acesso somente ao necessario, sem elevar a DLL; verificar primeiro permissao para usar esse ambiente. **Nao desligar firewall/Defender nem remover verificacao de token**.

A regra **Block TCP inbound para Sentrium.CloudLibLab.exe foi instalada/validada antes deste diagnostico** e nao foi alterada pelo `DiagnoseToken`. Como regra de bloqueio, nao abre porta; pode permanecer ate o proximo LAB, mas continua exigida validacao de inacessibilidade LAN por segundo host. Caso operador interrompa o projeto, remover com `scripts/windows/sentrium-cloud-lab-firewall.ps1 -Action Remove` apos encerrar o LAB. Isolamento e login NetSDK continuam NOT RUN. Codigo C++ e main/F004 intactos, PR4 draft.


## UAC ENABLED + STANDARD Windows real — liberado gate de privilege (29/10/2026)

O operador habilitou o UAC, reiniciou o Windows e executou `-Action DiagnoseToken` **localmente**, sem `git pull` (porque o Git recusou checkout/atualizacao por dubious ownership). Resultado REAL: `CURRENT_TOKEN=STANDARD`, `UAC=ENABLED`, `BUILTIN_ADMIN_ACCOUNT=NO`, `BUILTIN_ADMIN_APPROVAL_MODE=DISABLED`; a ultima flag pertence apenas ao Administrator interno, que NAO e a conta atual. Portanto o **gate de token limitado PASSOU**. Nenhuma DLL vendor executada nesta verificacao e nao houve nova alteracao do firewall.

**Problema Git independente:** apos UAC habilitado, `git pull` da pasta `C:/Projetos/SENTRIUM VMS/sentrium-vms` foi recusado por `fatal: detected dubious ownership` (pasta possuida por `BUILTIN/Administradores`, processo atual executa com usuario comum). NAO configurar excecao global `safe.directory=*`, NAO alterar propriedade/ACL automaticamente. O script de isolamento JA existia na copia local do usuario desde pull de fc409df6, portanto **nao e preciso dar pull novamente antes do proximo teste**. Se Git for necessario depois, preferir escopo por invocacao e auditar permissao/propriedade.

**Proximo passo REAL:** executar, sem elevar PowerShell, `.\scripts\windows\sentrium-simnext-cloud-lab.ps1 -Mode IsolationProbe`. O wrapper checa `-Action Status` de regra temporaria ja instalada e ABORTA antes de carregar DLL se ela nao estiver ativa (por exemplo, apos reinicializacao). Se passar, pede selecao da DLL original, Cloud ID duas vezes oculto e porta do gravador; testa TCP localhost sem NetSDK e mostra porta LAN temporaria; exige teste de outro computador LAN `Test-NetConnection <IP_DO_PC_LAB_NA_LAN> -Port <PORTA_TEMPORARIA> -InformationLevel Quiet`, resultado esperado False. **Nao enviar IP local, UUID ou senha ao chat**. Pressionar ENTER ao finalizar aciona `finally` e solicita UAC para remover a regra (aceitar). Se segundo PC nao disponivel, registrar LAN NOT RUN e nao avançar para NetSDK. Se `Status` falhar, nao repetir Prepare cegamente: verificar erro e se regra continua no ActiveStore. PR4 draft, main/F004/C++ intactos.

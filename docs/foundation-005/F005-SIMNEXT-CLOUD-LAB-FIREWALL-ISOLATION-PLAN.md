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

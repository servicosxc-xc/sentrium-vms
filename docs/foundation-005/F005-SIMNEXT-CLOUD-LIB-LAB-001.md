# F005 — LAB-001: bibliotecas Cloud locais do SIM Next (prova isolada)

**Estado:** codigo de laboratorio ADICIONADO; build Windows e execucao AINDA NAO realizados. Nada foi testado em um DVR por este novo harness.

## Por que este teste?

No legado XC Video IA, tres provas documentadas de `VVEyeAdapter(null,null)` + `AddSDKPort` conseguiram abrir tunel Cloud/Relay com porta TCP local: Cloud isolado 37792, NetSDK+Cloud 37799 e ASP.NET+Cloud 37806. Esses componentes foram carregados a partir de uma instalacao existente do SIM Next, NAO do Windows limpo. A rota Direct nao foi demonstrada nesses probes.

O usuario autorizou explorar experimentalmente o uso das bibliotecas em 28/10/2026 para levantar evidencias antes de solicitar formalizacao. Isso NAO substitui permissao de uso/redistribuicao; a licenca SIM Next pode limitar reutilizacao fora do produto.

## Componentes e separacao

- `SIMNext.P2PSDK.dll`, `SIMNext.Core.dll`, `SIMNext.Model.dll`, `Libt2uWrapper.dll` (.NET), `libt2u.dll` (nativa): **somente do diretorio original da instalacao**; NAO copiar, anexar a issue ou commitar.
- `tools/simnext-cloud-lab/Sentrium.CloudLibLab.csproj` + `Program.cs`: executavel .NET 10 x64 **de laboratorio**, nao componente do SENTRIUM; carrega DLLs em runtime apenas no modo Connect.
- `scripts/windows/sentrium-simnext-cloud-lab.ps1`: modo INSPECT padrao oferece seletor local para DLL principal, confirma cinco arquivos e assinaturas com `Get-AuthenticodeSignature`, valida SDK dotnet 10, roda apenas metadados. Nao requer serial ou credenciais na etapa INSPECT.
- `tools/simnext-cloud-lab/NuGet.Config`: desabilita fontes de NuGet online; projeto sem dependencias externas.

No modo `Connect` (somente apos resultado INSPECT e avaliacão dos limites de uso): operador digita `CONFIRMO`, o Cloud ID de gravador proprio/autorizado e porta de servico de fato configurada (nao assumir 37777 ou 37000). O programa instancia VVEyeAdapter com construtor de duas dependencias nulas conforme provas historicas, chama `AddSDKPort`, `P2pIsActive`, `RelayConnection`, inspeciona porta TCP local/escopo de bind, aguarda ENTER e chama `DeleteSDKPort` e `Cleanup`. DeviceId local efemero aleatorio, nao serial/UUID; a interpretacao definitiva desse ID nao foi publicada. **Nem executar se a licenca de teste nao permitir**. Se houver listener nao exclusivamente loopback, encerra e nao recomenda usar NetSDK, pois o legado ja exibiu 0.0.0.0 em uma observacao.

Os testes validariam somente: integridade/conjunto da biblioteca, compatibilidade TFM/ABI, criacao de tunel e reporte de estado. **NAO** comprovam login NetSDK com DVR, video, falta de dependencia do SIM Next instalado, permissionamento Cloud, licenca de redistribuicao, resiliencia, conexao Direct nem sucesso na infraestrutura da Intelbras. O P2P pode depender dos dados legitimos carregados das configuracoes do aplicativo. Nenhuma configuracao/chave/token sera extraida ou registrada.

## Passo unico de INSPECT no Windows

Na raiz da branch F005:

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"; git pull --ff-only origin foundation/005-device-connectivity; if ($LASTEXITCODE -eq 0) { .\scripts\windows\sentrium-simnext-cloud-lab.ps1 -Mode Inspect }
```

Selecionar `SIMNext.P2PSDK.dll` da instalacao local. Nenhum IP, Cloud UUID, credencial ou trecho de arquivo privado deve ser enviado ao GitHub/chat. Compartilhar apenas linhas BINARY/METADATA/INSPECT sem caminhos locais sensiveis.

**Proximos gates:** (1) INSPECT Windows; (2) revisar assinatura/origem/licenca e compatibilidade sem copiar as DLLs; (3) CONNECT de laboratorio apenas com dispositivos do proprio operador e permissao aplicavel; (4) eventual login NetSDK no localhost via um adaptador **isolado e condicionado**; (5) licenca comercial e direito de redistribuicao antes de integrar a release. Main/F004 Human Review intactos, PR #4 draft.


## INSPECT — Windows real PASS (28/10/2026)

Recebido log de Visual Studio 2022 Developer PowerShell no PC do operador, apos `git pull --ff-only` de `0b0cdf1`. O script foi executado com `-Mode Inspect`, localizou os cinco componentes **no diretorio de instalacao** e completou a compilacao/execucao do harness .NET 10 x64 (se nao houvesse runtime .NET e compilacao suficientes, o comando dotnet run nao teria produzido `INSPECT PASS`). Saida observada:

| Arquivo | Signature status | PE machine | Assembly kind |
|---|---|---|---|
| SIMNext.P2PSDK.dll | Valid | AMD64 | managed SIMNext.P2PSDK |
| SIMNext.Core.dll | Valid | AMD64 | managed SIMNext.Core |
| SIMNext.Model.dll | Valid | AMD64 | managed SIMNext.Model |
| Libt2uWrapper.dll | NotSigned | I386 | managed Libt2uWrapper |
| libt2u.dll | NotSigned | AMD64 | NATIVE |

Inspecao retornou duas linhas `INSPECT PASS`; **nenhum binario vendor foi carregado no modo INSPECT** (somente metadados), nenhum pedido Cloud foi iniciado e nenhum Cloud UUID/senha foi solicitado. O nome PE I386 de um assembly gerenciado pode significar AnyCPU IL e NAO comprova sozinho necessidade de processo 32-bit; avaliar CLR CorFlags ou comportamento de carga antes de concluir incompatibilidade. `NotSigned` pode ser normal para determinados binarios, mas precisa avaliacao de procedencia e integridade e nao prova que redistribuicao seja permitida. Nunca commitar hash nem copiar as DLLs proprietarias.

**Proximo gate: LAB-CONNECT-PENDING**, avaliado apenas no ambiente autorizado do operador e se permitido pelos termos de uso aplicaveis. Este teste executa codigo de bibliotecas proprietarias e pode contactar infraestrutura Cloud, diferente de INSPECT. Deve exigir confirmacao interativa explicita; nao solicitar UUID/serial/credencial na conversa. O usuario pode encerrar sem execucao antes de digitar CONFIRMO. No script, selecionar a mesma DLL da instalacao original, informar o Cloud ID/UUID do proprio gravador e a Porta de Servico correta (37000 foi apenas um exemplo historico de outro teste); nao assumir 37777. Relatar somente marcadores anonimizados `ADAPTER_READY`, `TUNNEL`, `LOCAL TCP LISTENER`, `LAB FAIL stage=` e eventuais linhas de erro, removendo qualquer dado sensivel. Mesmo PASS do tunel nao seria NetSDK login, vídeo ou autorizacao de redistribuicao. Produto SENTRIUM C++ permanece inalterado, PR draft, main/F004 intactos.


## LAB-CONNECT-001 — resposta real Windows; AUTO-STOP pelo escopo de bind (28/10/2026)

O operador executou `-Mode Connect` na instalacao local, confirmou interativamente e inseriu o Cloud ID de um gravador autorizado e sua porta de servico **somente no console**. O identificador do dispositivo apareceu no texto enviado ao chat, mas **nao sera reproduzido nos arquivos de pesquisa, handoff, log ou GitHub**. A porta de servico foi fornecida pelo proprio operador, sem inferir uma constante de produto.

Sequencia do log: bibliotecas presentes, assinaturas identicas a INSPECT, `ADAPTER_READY`, `TUNNEL PORT_RETURNED`, `TUNNEL STATE: ACTIVE`, `TUNNEL ROUTE: NOT_RELAY_REPORTED`, `LOCAL TCP LISTENER: OBSERVED`, `BIND SCOPE: NOT_PROVEN_LOOPBACK`, `SAFETY STOP`, e o wrapper terminou com `exit=9`. **Isto e resultado parcial POSITIVO de infraestrutura:** o wrapper conseguiu operar as bibliotecas em processo .NET independente da interface UI do SIM Next e a biblioteca reportou atividade/porta. **NAO comprova** autenticacao NetSDK, que o listener e exclusivo deste processo, que P2P Direct ocorreu, que pode usar por serial sem SIM Next instalado, que a conexao entrega bytes ou que existe autorizacao comercial. `NOT_RELAY_REPORTED` pode significar estado desconhecido; nao assumir Direct.

A interrupcao foi especifica de seguranca: porta observada mas ao menos um endpoint TCP nao era exclusivamente loopback, ou o conjunto estava ambiguo. No Windows, `GetActiveTcpListeners()` lista endpoints do sistema, nao PID e nao demonstra firewall. A condicao `listeners.All(IPAddress.IsLoopback(...))` foi falsa. O codigo manteve a restricao: **NAO** tentar NetSDK login na porta nem remover firewall; `finally` tenta `DeleteSDKPort`/`Cleanup`. O resultado `exit=9` e uma reprovacao do **gate de exposicao**, nao um erro `AddSDKPort`.

### Hotfix diagnostico LAB-001 r1

Atualizou-se `tools/simnext-cloud-lab/Program.cs` para imprimir apenas **categorias de bind**, sem IP: LOOPBACK, WILDCARD_IPV4, WILDCARD_IPV6, SPECIFIC_NON_LOOPBACK_IPV4 ou IPv6; mantendo `BIND SCOPE: NOT_PROVEN_LOOPBACK` e auto-stop. O Cloud ID passa a ser inserido em terminal **sem eco** via `Console.ReadKey(intercept:true)` (o primeiro laboratorio usava Console.ReadLine e o texto acabou no scrollback). Nenhum dump de endereco/serial/chave e produzido. Ainda nao e prova de atribuir o listener ao processo DLL, pois classificacao usa tabela global do Windows.

**Novo passo focal:** executar novamente `git pull` + `sentrium-simnext-cloud-lab.ps1 -Mode Connect` com o mesmo arquivo selecionado e dados privados digitados apenas no PC; encaminhar somente `BIND CATEGORIES`, `BIND SCOPE` e marcadores de tunel (sem ID). Se WILDCARD ou interface de rede, manter STOP e definir estrategia de isolamento testada antes de NetSDK. Nenhum firewall/NAT/Defender sera alterado automaticamente. C# alterado, compilacao/novo run Windows **NOT RUN** apos hotfix; somente inferencia a partir do log anterior. Main, F004 Human Review e C++ produto intactos, PR #4 draft; R10 C++ 6/6 synthetic lab e ultima evidencia native.

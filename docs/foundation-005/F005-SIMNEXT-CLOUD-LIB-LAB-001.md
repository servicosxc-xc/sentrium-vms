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

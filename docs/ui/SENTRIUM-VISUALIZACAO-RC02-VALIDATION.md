# SENTRIUM VMS — Visualização RC02 / AutoRegistro Recovery

**Congelamento para teste:** 2026-11-03. **Destino:** `release-candidate/visualizacao-r25-rc02`, derivada da feature `feature/r25-ui-parity`. RC01, F005 base e `main` preservadas.

## Escopo da RC02

1. **Registro Automático:** encaminhamento dos callbacks nativos de desconexão/retorno; registro de sessão offline; reautenticação por `EM_LOGIN_SPEC_CAP_SERVER_CONN` com o ID já cadastrado e endpoint observado; repetição via Worker a cada 15 s para sessões pendentes; remoção de sessões antigas sem travar para sempre; proteção contra logout manual.
2. **Mosaico:** captura de posições e tipo Extra/Principal ao perder conexão; recuperação após login NetSDK real com canais enumerados; suporte do Worker até 64 slots.
3. **Interface da Visualização r25:** árvore de locais/dispositivos/câmeras, abas junto à marca, apenas Home fixa, lateral com pesquisa de cor clara, grades e destaques, menus contextuais, selos IA/EVENTOS, drag/drop de câmeras e dispositivos.
4. **Correções pós-RC01:** botão `⋯` em cada tile e popup nativo para menu; arrastar tile pelo cabeçalho até outro tile usando posição do ponteiro, sem depender de DropArea sobre janela HWND nativa; correção do qmlcachegen C4702 isolada.

## Restrições e evidências

- O manual NetSDK distingue três modos de login e documenta que **auto registration login não oferece reconexão automática nativa**. Por isso há retry pelo aplicativo. Um último endpoint observado não garante reconexão se o firmware parar completamente de se registrar ou trocar de porta sem novo callback.
- O cadastro atual de dispositivos/credenciais/grupos é **em memória**, não banco persistente. A RC02 não resolve automaticamente retomada de cadastros ao encerrar e reabrir o processo sem recadastramento, caso esse comportamento seja requerido.
- Não incluir credenciais, ID completo, endpoint remoto ou IP privado em logs compartilhados.
- `IA` ativo depende de evidência do backend; `EVENTOS` é seleção local até feed real. Sem dados fictícios.
- **Qt/MSVC build, inicialização da GUI e reconexão em NVR no Windows: NÃO VERIFICADOS na preparação da RC02.** O teste do operador é obrigatório antes de aprovar/mesclar.

## Teste na máquina Windows

Executar no **Developer PowerShell VS2022 x64**, com aplicativo fechado e árvore de trabalho limpa:

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
if (git status --porcelain) { throw "Há alterações locais; interrompendo checkout." }
git fetch origin release-candidate/visualizacao-r25-rc02
if ($LASTEXITCODE -ne 0) { throw "Falha no fetch" }
git show-ref --verify --quiet refs/heads/release-candidate/visualizacao-r25-rc02
if ($LASTEXITCODE -eq 0) {
    git switch release-candidate/visualizacao-r25-rc02
} else {
    git switch -c release-candidate/visualizacao-r25-rc02 --track origin/release-candidate/visualizacao-r25-rc02
}
if ($LASTEXITCODE -ne 0) { throw "Falha no checkout" }
git pull --ff-only origin release-candidate/visualizacao-r25-rc02
if ($LASTEXITCODE -ne 0) { throw "Falha ao atualizar" }
.\scripts\windows\sentrium-mosaic-build-run.ps1
```

### Gate único de validação

1. Confirmar **MOSAIC BUILD PASS**; em falha, obter `out/sentrium-release/sentrium-mosaic-build-last.log`.
2. Abrir Registro Automático no SENTRIUM e usar o mesmo IP/porta e ID do NVR; aguardar status `Connected` e canais reais.
3. Abrir dois canais, um Extra e um Principal; anotar os tiles. Interromper a conectividade **reversivelmente** (não modificar ID/host/porta nem salvar a configuração de registro no NVR).
4. Restabelecer conectividade; observar `Connecting` → `Connected` e retorno de canais/tiles. Repetir duas vezes e confirmar que nenhum outro NVR foi desconectado.
5. Reiniciar somente o aplicativo (sem resalvar config do NVR): verificar a limitação atual de cadastro não persistido e se o equipamento volta a emitir callback ao novo listener.
6. Conferir botão `⋯`, menu Principal/Extra, arrastar tile 1→4 vazio e trocar tile 1↔4 ocupado pelo cabeçalho.
7. Verificar abas no topo sem atalhos fixos, árvore compacta, pesquisa legível e layouts 1/4/9/16; aumentar layouts maiores apenas conforme recursos do PC.

**Status esperado depois da preparação:** RC02 publicada como **fonte para compilação local**. Não equivale a executável binário distribuído ou aprovação funcional.

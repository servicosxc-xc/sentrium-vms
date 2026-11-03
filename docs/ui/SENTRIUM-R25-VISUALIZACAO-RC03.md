# SENTRIUM VMS — Visualização r25 RC03 (código-fonte para teste)

**Data:** 03/11/2026. **Autorização:** operador aprovou consolidar melhorias e disponibilizar uma nova versão após capturas comparativas de UI. **Destino:** `release-candidate/visualizacao-r25-rc03`, separada da RC01, RC02, F005 e main.

## Mudanças incluídas desde a RC02

1. **Cadastro de dispositivos → Grupo/local:** campo `ComboBox` editável, mostrando `SEM GRUPO` + grupos registrados, permitindo digitar novo nome. LAN/IP e Registro Automático recebem o grupo no cadastro, na mesma transação C++. Todos os dispositivos sem grupo são exibidos em `SEM GRUPO`.
2. **Prevenção de duplicidade:** grupos normalizados por `QString::simplified()`, comparação case-insensitive com reaproveitamento da grafia existente, proibição de separadores de caminho e caracteres não imprimíveis, limite 80 caracteres e reserva de `SEM GRUPO`. LAN impede cadastro com o mesmo IP/host + porta; Registro Automático já impedia o mesmo ID exato. Os registros de dispositivos ainda ficam em memória conforme Foundation 005.
3. **Menus da árvore sobre o vídeo:** todos os menus de Grupo/Dispositivo/Câmera e submenus usam `popupType: Popup.Window`, com tema escuro. Assim abrem em uma janela separada, acima dos HWND nativos de vídeo (sem tentar usar `z` de itens QML, que não resolve empilhamento do `WindowContainer`). O menu de câmera no mosaico permanece separado como `Popup.Window`.
4. **Aproveitamento total do tile:** removidos cabeçalho fixo e barra inferior. `WindowContainer` passa a ocupar toda a área de cada tile. Nome da câmera, estado de IA real, seleção EVENTOS, stream Extra/Principal e fechamento ficam em **uma única barra flutuante**, que aparece apenas quando o cursor passa sobre o tile. O status detalhado continua disponível por tooltip. Para evitar cobrir vídeo nativo com Item QML (invisível nesse cenário), a barra é implementada por **um único Popup.Window reutilizado** para todo o mosaico, reposicionado pelo cursor a cada 80 ms, sem criar 64 janelas auxiliares. Não alterar video encode, buffers ou NetSDK.
5. **Pendências de interação RC02 incluídas:** entrada Win32 do NetSDK `QWindow`/HWND e child HWND encaminhada a QML para clique direito; arraste tile→tile via rastreamento do cursor e `swapLiveSlots`, independentemente do QML receber release sobre a imagem; sem botão extra `⋯`; seta da árvore sem fundo. Recuperação de Registro Automático/streams da RC02 preservada sem novas modificações neste lote.

## Limitações declaradas

- Os **grupos, vínculos, dispositivos e favoritos ainda são mantidos em memória** nesta Foundation. A lista de grupos não será conservada entre reinícios da aplicação sem futuro módulo DataRoot/persistência. A UX da RC03 permite organizar dispositivos enquanto o aplicativo está aberto.
- Menus e HUD em `Popup.Window` são compatíveis com Qt 6.12 em Windows, mas seu posicionamento e a reação da janela nativa ao passar o mouse **precisam ser validados em hardware real**. Múltiplos monitores/DPI exigem teste específico.
- IA e Eventos ao Vivo ainda não estão conectados a um motor real; não mostrar processamento fictício. A faixa só mostra IA se `aiState` vier como `Active`.
- O Registro Automático **pareceu melhorar** no teste da RC02, mas ainda não há confirmação com ciclos repetidos e reinício dos componentes.
- **Nenhuma compilação Qt/MSVC ou teste operacional da RC03 foi executada pelo assistente**. A validação está pendente do operador Windows. Não fazer merge em main antes do gate.

## Gate de validação RC03

1. Compilar com `scripts/windows/sentrium-mosaic-build-run.ps1` (Visual Studio 2022 x64 Developer PowerShell, Qt 6.12.0, SDKs NetSDK/PlaySDK locais, SENTRIUM fechado).
2. Cadastrar NVR novo sem selecionar grupo: confirmar `SEM GRUPO`. Cadastrar outro no grupo digitado `Portaria`; cadastrar um terceiro em `portaria`: o grupo deve continuar único, com ambos sob `Portaria`. Testar seleção do dropdown, prevenção de IP:porta duplicado e de ID de AutoRegistro repetido.
3. Com múltiplas câmeras abertas, clicar com botão direito em dispositivos e câmeras na árvore: submenus devem ficar por cima do vídeo sem serem cortados pelo mosaico.
4. Mosaico 4 e 9: vídeo preenche 100% do tile sem barra fixa; mover cursor para dentro do tile mostra HUD discreta em cima da imagem; afastar cursor oculta HUD; nome, Extra/Principal, IA/EVENTOS quando aplicáveis, fechar tile via HUD; clique direito sobre a imagem apresenta o menu.
5. Arrastar pelo vídeo um tile ocupado para vazio e outro ocupado, confirmando movimento/troca e continuidade do stream. Repetir com tiles pequenos e telas em DPI 100% e, se possível, escalonamento diferente.
6. Cortar/repor rede de NVR inscrito em AutoRegistro sem resalvar configuração; verificar reconexão e restauração do mosaico duas vezes.
7. Se houver erro, usar o log `out/sentrium-release/sentrium-mosaic-build-last.log` e a mensagem de QML; nenhum token/credencial/ID completo deve ser enviado.

## Como executar

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
if (git status --porcelain) { throw "Alterações locais detectadas. Pare antes do checkout." }

$branch = "release-candidate/visualizacao-r25-rc03"
git fetch origin $branch
if ($LASTEXITCODE -ne 0) { throw "Falha no fetch RC03" }

git show-ref --verify --quiet "refs/heads/$branch"
if ($LASTEXITCODE -eq 0) {
    git switch $branch
} else {
    git switch -c $branch --track "origin/$branch"
}
if ($LASTEXITCODE -ne 0) { throw "Falha no switch RC03" }

git pull --ff-only origin $branch
if ($LASTEXITCODE -ne 0) { throw "Falha ao atualizar RC03" }

.\scripts\windows\sentrium-mosaic-build-run.ps1
```

**Status:** RC03 será publicada como branch de código para compilação local; não equivale a EXE distribuído, build validado ou merge definitivo.

# SENTRIUM VMS RC03 — incidente de UI evidenciado em gravação

**Origem:** gravação de tela enviada pelo operador em 03/11/2026, duração 58,55 s, 1920×1080/60 fps, da `Visualização r25 RC03 (teste)`.

## Evidência observável no vídeo

- Em torno de 05s/23s/31s, a barra flutuante exibe textos de posição de câmera (`Canal 1`, `Canal 6`, `Canal 8`) sobre tiles sem imagem; não se restringe às câmeras abertas e aparece como uma faixa deslocada/inadequada.
- Em torno de 40s–49s, o menu da câmera abre com fundo escuro e fontes quase pretas, tornando as ações ilegíveis; a posição visual não segue intuitivamente a interação com a câmera.
- Há câmeras exibindo frames reais do NVR no mesmo mosaico. A gravação **não comprova falha da autenticação NetSDK**, nem evidencia que a rede do NVR falhou. Os problemas comprovados são de HUD, composição e interação.
- O vídeo não prova que o movimento do tile foi concluído nem que a troca de stream foi recebida corretamente: evitar declarar drag/drop aprovado.

## Diagnóstico no código

A RC03 introduziu `Popup.Window` reutilizado (`tileHoverHud`), reposicionado a cada 80ms pelo cursor global, junto com o gerenciador de drag/drop e outros popups nativos do Qt. Isso cria janelas top-level transitórias em cima de `WindowContainer`/NetSDK; não permite garantir visual estável nem input dentro do tile. Além disso, a árvore e menu de câmera receberam cores de background escuras sem controlar consistentemente o foreground dos `MenuItem`, reproduzindo o texto ilegível. Em `CameraContextMenu` havia escolha posicional manual que não respeitou o uso confortável do contexto.

## Contenção aplicada exclusivamente em feature/r25-ui-parity

1. **Removidos** `tileHoverHud` e o `Timer` de 80ms que abre/fecha o popup conforme hover.
2. Restaurada uma **faixa compacta de 16–19px** na parte superior de cada tile, sem rodapé, mantendo reprodução nativa e comandos por menu. É um **recuo deliberado** do requisito de barra hover para recuperar previsibilidade e preservar a maior parte da imagem.
3. Alterada abertura do `CameraContextMenu` para `popup()` sem coordenadas manuais; conforme Qt 6.12, abre na posição real do cursor no desktop. `popupType: Popup.Window` permanece para sobrepor `WindowContainer`.
4. Restaurados foreground/background padrão dos menus da árvore e câmera, mantendo `Popup.Window`, para impedir texto preto em background escuro.
5. Clique esquerdo sobre vídeo nativo **não inicia drag automaticamente**; mantém seleção. Arrastar por enquanto é somente pela faixa compacta, para evitar swap acidental por movimento normal do cursor.
6. **Preservados** as correções de registro automático RC02, cadastro por grupos e prevenção de duplicidade. Nenhuma alteração em NetSDK, Worker, protocolos ou credenciais nesta contenção.

## Restrições e validação

- Código alterado apenas em `feature/r25-ui-parity`. **RC03 publicada, RC02, F005, main não foram modificadas.** Não gerar RC04 sem autorização do operador.
- Fonte atualizada e verificação estática de blocos/refs passaram. **Não houve build Qt 6.12/MSVC Windows após correção**, nem teste de runtime.
- A meta da barra hover fica **adiada**: uma sobreposição genuína de 100% sobre vídeo exige uma integração estável de composição/overlay com o renderizador, e não mais popups temporários por frame.
- Para operação imediata antes de nova homologação, usar a RC02, que manteve o backend de reconexão aprovado preliminarmente pelo usuário, ciente das limitações de interface já relatadas.

Próxima fase: criar plano de reprodução e teste automatizado de menu/drag em Windows e arquitetura de render para overlay; não misturar mudanças de UI de alto risco com reparos NetSDK em uma única liberação.

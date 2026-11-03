# UI r25 — alterações 2, 3 e 4: cabeçalho, árvore e layouts

**Origem:** capturas de tela do operador e solicitações diretas na conversa de 31/10/2026.
**Branch:** \`feature/r25-ui-parity\` / PR #5 draft. **Sem release ou merge e sem aprovação visual ainda.**

## Alteração 2 — abas no cabeçalho

- Abas movidas para a mesma barra superior, **depois do ícone/nome SENTRIUM** e antes de Configurações/Ready/versão.
- A segunda barra horizontal de 40px foi removida para devolver área útil à Visualização.
- Abas fecháveis (exceto Home) mantêm os métodos de navegação anteriores.
- O \`×\` usa \`ToolButton.background: Item {}\` para ficar sem retângulo preenchido e mudar apenas a cor do glifo quando passar o mouse.
- Rolagem horizontal permite novas abas sem deslocar título/status; **não** implementa sessões de vídeo independentes (pendência já documentada).

## Alteração 3 — lateral apenas pesquisa

- Removidos da lateral de Visualização: título DISPOSITIVOS, botão + Adicionar, bloco status/Registro Automático, link Abrir, bloco de erro, cabeçalho GRUPOS E DISPOSITIVOS, botões visíveis Recolher/Expandir e estrela de filtro.
- \`DeviceExplorer.qml\` mantém **somente uma barra de pesquisa acima da árvore**. A árvore compacta Grupo → DVR/NVR → Câmera continua abaixo.
- As ações continuam acessíveis no **menu de contexto (botão direito)** de cada grupo/gravador/câmera. No grupo: novo grupo, expandir/recolher todos, filtro de favoritos. No gravador: mover de grupo, abrir todos, conectar/desconectar/remover. Na câmera: Extra/Principal e favoritos.
- Cadastro permanece no módulo Dispositivos (\`+ Novo\`); configuração do listener no módulo Configurações. Nenhuma função de NetSDK eliminada.
- Lateral padrão ajustada de 310px para 268px; largura mínima 205px.

## Alteração 4 — layouts padrão e destaque

- Barra rápida: 1, 4, 9, 16 e menu **Todos os layouts**.
- Popup: grades **1, 4, 9, 16, 25, 36, 49, 64**; destaques **6, 8, 10, 12, 14**.
- Grades: NxN. Destaques: câmera principal grande à esquerda e demais câmeras em grade à direita; não são as proporções exatas de cada template da r25, pois a imagem fornecida mostra o menu, não todos os templates.
- \`DeviceController\` ampliado de 16 a 64 slots e validando exatamente as 13 capacidades. \`DeviceWorker\` continua gerenciando handles por slot sem limitação fixa.
- \`Main.qml\` usa posicionamento de tiles por coordenadas, dimensões/span, sem trocar a capacidade para maximizar um tile. \`WindowContainer\` agora é carregado por \`Loader\` **somente para tiles ocupados**, evitando alocar QWindows em espaços vazios.
- \`openRecorder()\` abre até 64 canais, aumentando automaticamente para a próxima grade somente quando necessário, usando a modalidade Extra.
- Indicadores e menu mantêm limitações reais: **64 streams simultâneos, FPS, CPU/GPU, latência, reprodução visual e teardown ainda não foram verificados em Windows**. Os layouts não equivalem a benchmark aprovado; máximo prático depende de hardware, streams e dispositivo.

## Situação e gate

- **Versão gerada:** NÃO. O usuário quer revisar as mudanças do módulo e confirmar antes de liberar uma build.
- **Código:** atualizado apenas em \`feature/r25-ui-parity\`.
- **Build Windows/QML:** NOT RUN. Análise estática de estrutura e algoritmo geométrico sem sobreposição PASS; isso NÃO comprova carregamento QML/RealPlay.
- **Sem alteração:** branch F005 base, \`main\`, F004, protocolos Cloud e autenticação NetSDK/AutoReg.
- Ao validar futuramente: abas ao lado do nome, close X sem fundo, search-only, abertura via menu de contexto, todas as opções do popup, mudança de grid/destaque sem travar, e hardware real com 1/4/9/16 antes de testar 25–64 de forma gradual.

# UI — Árvore de Dispositivos (Visualização) — R1

**Solicitação:** usuário forneceu capturas do SENTRIUM atual e do XC Video IA r25. A coluna esquerda do SENTRIUM com cartões de DVR/NVR deve ficar **em árvore compacta**, semelhante à r25: **site/grupo → gravador → canais**.

**Branch:** `feature/r25-ui-parity`; PR #5 draft. **Não é release**. A nova versão/teste Windows será preparada somente após confirmação do operador sobre a interface.

## Entrega codificada

- `DeviceExplorer.qml` deixa de exibir cartão por gravador e passa a uma árvore virtualizada `ListView` de 3 níveis, ordenada por grupo e dispositivo. O estado inicial mantém os grupos abertos e os gravadores recolhidos.
- Ícone, setas de expansão/recolhimento, pesquisa por site/gravador/câmera, favoritos temporários e menus de contexto. Com pesquisa/favoritos, os nós correspondentes expandem automaticamente sem alterar a preferência normal de expansão.
- Botão `+ Grupo` cria grupo de organização, menu do gravador `Mover para grupo...` permite classificar equipamentos. Dispositivos não classificados ficam em **Sem grupo** (nome neutro, nenhum site da captura importado).
- `DeviceController.hpp/.cpp`: propriedades `deviceGroups` e `group` de cada dispositivo; ações de criação de grupo e movimentação do vínculo, **exclusivamente metadados locais em memória**. Validação de nome, unicidade case-insensitive, bloqueio de nomes inválidos. Sem mudança de autenticação, registros, NetSDK, credenciais, handles e streams.
- `DeviceController::mosaicSlots()` agora expõe `deviceId` lógico + `channel` ao QML para distinguir câmeras abertas. Indicador verde é mostrado só após criação de handle NetSDK de vídeo (`Reprodução solicitada`), **não certifica imagem/movimento decodificados**. Dispositivo neutro; Cloud Direct azul/Relay amarelo apenas quando estado verificado; falhas vermelhas.
- Os comandos Extra/Principal, Conectar/Desconectar, Abrir todos e Remover permanecem direcionados ao mesmo `DeviceController`.
- `Main.qml`: liga árvore à lista `devices`, aos nomes `deviceGroups`, aos `mosaicSlots` e às ações de organização.

## Limites deliberados

- Nomes e grupos **não são persistidos** após fechar o cliente; primeira etapa é a aparência e o comportamento de árvore solicitados, sem criar arquivo de configuração fora do DataRoot. Favoritos também continuam somente em memória.
- A árvore permite **um nível de grupo/local** e dois níveis adicionais (gravador/câmera); subgrupos, renomear grupo, mover por arrastar/soltar, cadastrar dispositivo diretamente em grupo, persistência de grupos/favoritos e importação da r25 dependem de requisitos e confirmação posteriores.
- **Nenhum exemplo das capturas foi cadastrado artificialmente.** O NVR já cadastrado aparece em `Sem grupo` até o operador movê-lo.
- A tela `Dispositivos` de cadastro/diagnóstico continua tabela, pois a comparação enviada se refere à **árvore lateral de Visualização**.
- **Build/execução real Windows/Qt NÃO EXECUTADOS neste lote**; integridade textual e análise estática não equivalem a QML carregado. A UI ainda deve ser vista pelo usuário antes de ser considerada aprovada. AutoRegistro pendente de confirmação em equipamento novo.

## Checklist visual para aprovação

- Local/Site com setas, DVR/NVR sob grupo, canais sob DVR/NVR como na r25.
- Grupos abertos e DVRs recolhidos ao iniciar; expandir/recolher, busca e favoritos funcionam.
- Criar `Portaria` e mover NVR para lá, sem afetar conexão/streams; verificar agrupar corretamente.
- Canal só surge após NetSDK retornar a lista real, e sua seleção abre no mosaico. Estado não é falsificado.
- Nenhuma janela de Release, merge ou mudança em `main` antes da aprovação.

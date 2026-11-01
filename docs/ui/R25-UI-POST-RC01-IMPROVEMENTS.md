# SENTRIUM VMS — ajustes após RC01, melhoria 1 (01/11/2026)

**Solicitação do operador:** coletar novas melhorias, trabalhando na branch de desenvolvimento sem gerar nova versão até autorização expressa.

## Melhoria 1 — Cabeçalho e abas

- O usuário enviou duas capturas do RC01 e pediu remover o grande espaço vazio entre a marca/nome do software e a divisória das abas. Em `apps/sentrium-client/Main.qml`, o `ColumnLayout` da marca recebeu `Layout.minimumWidth`, `preferredWidth` e `maximumWidth` fixados em 187px, eliminando o crescimento indevido que afastava a divisão/abas para a direita.
- Removidos da faixa superior os dois atalhos fixos `+ Nova visualização` e `Reprodução`. **Somente Home permanece como aba inicial fixa**; as demais abas (Visualização, Dispositivos, Reprodução, Configurações etc.) aparecem dinamicamente quando o usuário as abre.
- Não foi excluída a funcionalidade de criar uma nova Visualização nem Reprodução: os respectivos atalhos continuam nos cartões do módulo Home, usando `window.openModule` existente.
- O botão `Configurações` do canto direito permanece, pois não foi incluído nas duas remoções solicitadas.
- Não foi alterado nenhum backend NetSDK/registro, mosaico, Cloud/P2P ou regras IA.

## Estado

**Aplicado apenas à branch `feature/r25-ui-parity`; a branch `release-candidate/visualizacao-r25-rc01` NÃO foi modificada.** Sem build/teste visual ou merge. Não criar versão, release ou incorporar à RC antes do usuário enviar todas as melhorias e autorizar explicitamente. A revisão do posicionamento exato será feita na próxima versão aprovada para teste.

## Melhoria 2 — contraste da pesquisa de câmeras

- O operador identificou o texto da pesquisa lateral com cor preta, ilegível no fundo escuro.
- Em `apps/sentrium-client/DeviceExplorer.qml`, `cameraSearch.placeholderTextColor` passa a usar `explorer.textPrimary` (`#eef2f7`), a mesma cor do texto da aba Home ativa. A cor do texto digitado já utilizava essa propriedade.
- Ajuste somente na branch `feature/r25-ui-parity`, sem alterar a versão RC01. **Sem nova build/release até autorização.**

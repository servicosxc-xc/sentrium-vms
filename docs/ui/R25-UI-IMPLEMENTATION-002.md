# SENTRIUM VMS — UI-001 / UI-002 / UI-003, paridade r25 (31/10/2026)

**Branch isolada:** `feature/r25-ui-parity` (PR #5 draft).
**Base:** `foundation/005-device-connectivity`, commit `afdc8db`.
**Estado:** código Qt/QML versionado; **não compilado ou executado no Windows neste lote**. Não confundir inspeção estática com PASS.

## Entrega real no código

- `apps/sentrium-client/HomeDashboard.qml`: módulo Home separado, atalhos compactos, operação/status baseado no controller verdadeiro e mensagens explícitas para recursos indisponíveis.
- `apps/sentrium-client/DeviceExplorer.qml`: explorador de câmeras com filtro por nome/canal, recolher/expandir, estrela (favoritos **somente em memória**), menu de dispositivo e câmera (Extra/Principal), abrir todos os canais, conectar/desconectar/remover por sinais encaminhados ao controller existente.
- `apps/sentrium-client/DevicesWorkspace.qml`: módulo independente de Dispositivos com tabela, busca, seleção, estados, diagnóstico e lista de canais com Extra/Principal. Edição desabilitada enquanto não existe backend/persistência; não fingir edição de credenciais.
- `apps/sentrium-client/Main.qml`: integra os dois novos módulos, mantém diálogo de cadastro, trabalhador de conexão e NetSDK intactos; cabeçalho do tile tem menu de seleção, maximizar/restaurar e limpar; duplo clique amplia/restaura um tile **sem chamar `setMosaicCapacity(1)` ou parar streams dos demais tiles**. O controller é redimensionado antes do Repeater do QML para evitar acesso momentâneo a slots ainda não criados.
- `apps/sentrium-client/CMakeLists.txt`: inclui os três novos QML na mesma `qt_add_qml_module`.

## Limitações e próximos gates

1. **Nenhuma linha de C++ NetSDK/Registro Automático/Cloud foi alterada.** A regressão do AutoRegistro vista em campo continua sem diagnóstico conclusivo; teste no novo dispositivo não foi relatado.
2. **Múltiplas abas de Visualização ainda compartilham o mesmo mosaico global**, não são sessões independentes. Corrigir no futuro com workspace por aba e handles/superfícies adequadas, antes de declarar paridade r25 de abas.
3. Favoritos são temporários (não há persistência DataRoot). Não há grupo/edit de dispositivos ou drag/drop nesta entrega, apesar de constarem da especificação; não apresentá-los como prontos.
4. `WindowContainer`/QWindow continua ponte provisória de render e não entrega frames ao processamento de IA/LPR. Maximizar visualmente não reabre stream nem troca para Principal automaticamente.
5. Os módulos Eventos/Mídia/Regras IA/LPR, demais Configurações e Playback NVR continuam sem backend. Não simular dados.
6. Validação necessária no Windows Qt 6.12/MSVC: obter `feature/r25-ui-parity`, rodar `scripts/windows/sentrium-mosaic-build-run.ps1` com SENTRIUM fechado; verificar QML module loading, Home, Dispositivos, navegação Visualização, menu/favoritos/expandir, Registro Automático com NVR real e Extra/Principal. Testar maximizar/restaurar sem interrupção de outros canais. Se erro, corrigir antes de merge.
7. F005 base, F004 e `main` **não foram alteradas**. PR #4 e PR #5 permanecem draft, sem Human Review aprovado.

## Continuar

Prioridade após Windows PASS da shell: fechar a lacuna de persistência e isolamento por abas, depois completar UX de Visualização (drag/drop, menus de contexto reais, grupo→gravador→câmera), com testes de teardown; avançar Eventos/Mídia e regras de IA/LPR por backend real. Ler `docs/ui/R25-UI-FUNCTIONAL-PARITY.md` e `docs/HANDOFF-SENTRIUM-CURRENT.md` antes de mexer em arquitetura/conexão.

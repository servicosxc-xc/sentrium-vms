# UI — paridade e decisão condicionada

Auditoria estática, sem captura de tela ou execução. O inventário lista os XAML/ViewModels individualmente; esta tabela agrupa os comportamentos observados. Uma opção visível não comprova uma operação implementada.

| Área | Comportamento r25 / lacuna | Requisito C++ |
|---|---|---|
| Shell | chrome próprio, 1440×900, mínimo 1080×680; Home, Visualização, Eventos, Regras IA, Mídia, Dispositivos, LPR, Configurações | navegação, múltiplas abas e descarte de assinaturas |
| Dispositivos | cadastro/edição/exclusão, conexão/desconexão, canais gerados Canal NN, árvore, abrir canal/dispositivo | manter fluxo; nomes/capacidades reais dependem SDK |
| Favoritos | coleção de sessão; persistência dedicada não demonstrada | preservar sessão; durabilidade somente requisito futuro explícito |
| Mosaico | grids 1/4/9/16/25/36/49/64; focos 6/8/10/12/14; troca e maximização | equivalência geométrica, DPI e ciclo de vida |
| Stream | Principal/Extra fixos; Automático promove maximizado a Principal e demais a Extra | reproduzir modo legado; não introduzir autotuning por CPU |
| Render | controles nativos permanecem vivos ao maximizar | medir recursos ocultos e política explícita de suspensão |
| Menu canal | fechar, fechar todos, stream, maximizar, inteligência, movimento, configuração/overlays e simulação têm handlers | testes de cada comando e transições |
| Menu pendente | snapshot contextual, instant-playback, áudio, talk, gravação manual, aspectos e outros chegam a mensagem de integração | manter como pendência; não declarar funções concluídas |
| IA | editor, geometria/ROI/linha, overlays, detalhes, movimento, eventos e ações | golden de coordenadas/estado; não executar inferência na UI |
| LPR | master, perfis, ROI/gate, leituras, revisão, histórico e evidências | mesmas regras por perfil, sem fila bloqueante no render |
| Eventos | lista/filtros/detalhes, ações manuais, PTZ, monitor | preservar filtros e contratos; abrir monitor não resolve ocorrência |
| Monitor | evidência e câmera ao vivo inclusive outra origem; seguir, topo, 15/30/60/120s ou manual | ciclo de vida, autorização e descarte |
| Mídia | evidência/snapshot e reprodução DAV por caminho próprio | não confundir com gravação/timeline de NVR |
| Ações | LivePopup/PTZPreset executados; e-mail só log; Webhook/AlarmOutput sem executor | comunicar capacidades; ações por regra/perfil, nunca globalizar |
| Configurações | sistema, storage, Muralha; preferência de raiz não move dados | v2-dev isolado, migração futura offline |
| Usuários/notificações | telas reservadas, autenticação/RBAC completos ausentes | backlog explícito Admin/Operator/Viewer/Configurator e escopos |
| Listas | whitelist/blacklist/personalizadas ausentes | futura busca, proprietário, veículo, validade, observações, auditoria e ações condicionais |
| Telemetria | CPU/RAM/GPU do servidor no cabeçalho; poll 2,5s, stale 10s; vídeo local 1s | distinguir host/processo/cliente; desconhecido não vira zero |

## Comparação de frameworks

| Critério | Qt 6.12 LTS Quick | Qt Widgets + render nativo | WinUI 3 C++/WinRT |
|---|---|---|---|
| Mosaicos/overlays | scene graph/RHI e integração textura D3D11; sincronização a provar | widgets tradicionais e superfície Direct3D dedicada | SwapChainPanel/DirectX; composição e interop a provar |
| UI rica/DPI | QML favorece composição; limitar objetos por frame | formulários maduros, mais trabalho para overlays fluidos | integração Windows, XAML e acessibilidade |
| Risco | threads scene graph, recursos GPU, licença dos módulos | janelas/handles, composição e DPI misto | Windows App SDK, packaging, C++/WinRT e ciclo de vida |
| Portabilidade | potencial multiplataforma, SDK Intelbras ainda Windows | potencial multiplataforma, mesma restrição | Windows |
| Licença | comercial ou LGPL elegível, módulos GPL separados | mesmos cuidados Qt | fontes WinUI MIT; redistribuição Windows App SDK separada |
| Evidência de performance local | não medida | não medida | não medida |

Recomendação **Proposed: Qt Quick**, condicionada a prova de mosaico 64 canais, textura D3D11, overlays, DPI misto, acessibilidade, 24–72h e aprovação de licença. Widgets é alternativa se integração nativa comprovar menor risco; WinUI é alternativa válida para compromisso Windows. Não há vencedor de desempenho demonstrado. Não contratar licença nem iniciar UI nesta auditoria.

Qt documenta textura D3D11 nativa; isso não garante importar NV12 multiplanar sem conversão/cópia. [API Qt](https://doc.qt.io/qt-6/qnativeinterface-qsgd3d11texture.html), [SwapChainPanel Microsoft](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.controls.swapchainpanel), [Qt 6.12](https://www.qt.io/blog/qt-6.12-released).
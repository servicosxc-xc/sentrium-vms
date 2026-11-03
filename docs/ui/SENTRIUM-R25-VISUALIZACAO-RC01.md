# SENTRIUM VMS — Visualização r25 RC01 (pré-compilação)

**Solicitação do operador:** em 31/10/2026, encerrou a coleta de alterações e autorizou aplicar o lote UI-001–UI-008 no aplicativo para uma nova versão de teste no Windows.

**Código-base:** branch `feature/r25-ui-parity`, PR #5, comparado com `foundation/005-device-connectivity`; não alterar nem mesclar `main` ou F004.

## Alterações incluídas

1. Árvore compacta site/grupo → gravador → canais na lateral da Visualização.
2. Abas no topo, próximas ao nome SENTRIUM, com X sem fundo; remoção da segunda barra.
3. Lateral apenas com campo de pesquisa e árvore; comandos remanejados para menus contextuais.
4. Grades 1/4/9/16/25/36/49/64 e destaques 6/8/10/12/14, com tiles nativos criados sob demanda.
5. Menu contextual da câmera com fechamento, visualização, stream Principal/Extra, favoritos, diagnóstico e itens futuros desabilitados.
6. Menu contextual de gravador com conectar/desconectar, abrir todos Extra/Principal, fechar canais por gravador e detalhes.
7. Indicadores IA e EVENTOS por tile; IA ativa exige confirmação do Server (Foundation 005 reporta Unavailable). EVENTOS selecionado é preferência visual, **sem feed real conectado**.
8. Arrastar câmera da árvore para tile, gravador para múltiplos tiles em Extra e reorganizar tiles abertos pelo cabeçalho.

## Limites honestos

- **Não é um executável compilado nem um release homologado.** A preparação é de código versionado e pré-voo estático; a compilação do MSVC/Qt 6.12 depende da máquina Windows com NetSDK/PlaySDK disponíveis.
- Ainda falta validar menu sobre vídeo `WindowContainer`, arrastar/soltar real, estabilidade de stream e layouts maiores que 16 com câmeras reais.
- Os dados de dispositivos, grupos, favoritos e preferências de Eventos permanecem em memória conforme Foundation 005; nada foi promovido como persistência.
- Cloud/P2P independente, IA/LPR, gravação, snapshot, áudio e Eventos ao vivo reais continuam dependentes de seus backends.
- Não promover PR #5 ou a versão candidata para a branch de produção sem build + avaliação do operador.

## Como validar no Windows

No **Visual Studio 2022 x64 Developer PowerShell**, sem administrador, com SENTRIUM fechado:

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git status --short
git fetch origin
git switch --track origin/release-candidate/visualizacao-r25-rc01
.\scripts\windows\sentrium-mosaic-build-run.ps1
```

Caso a branch local já exista, substituir `git switch --track ...` por `git switch release-candidate/visualizacao-r25-rc01` seguido de `git pull --ff-only`. **Não sobrescrever alterações locais.** O helper espera build/cache/toolchain já configurados, VS2022 x64 PowerShell, Windows com Qt/NetSDK/PlaySDK, e gera `out/sentrium-release/sentrium-mosaic-build-last.log` (os últimos 45 linhas são mostrados). O helper verifica se o processo não encerra nos três segundos iniciais, não prova funcionamento dos módulos.

Se falhar, enviar as 45 linhas finais do log e as mensagens de QML. Se iniciar, testar árvore, abas, menu e drag/drop com 1–4 streams e só depois aumentar a grade. Registrar a aprovação visual/funcional separadamente.

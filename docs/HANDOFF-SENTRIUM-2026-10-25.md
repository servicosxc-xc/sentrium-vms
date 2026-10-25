# SENTRIUM VMS — HANDOFF DE CONTINUIDADE

**Data:** 25/10/2026  
**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Repositório:** `servicosxc-xc/sentrium-vms`  
**Branch ativa:** `foundation/004-media-lab`  
**Workspace local:** `C:\Projetos\SENTRIUM VMS\sentrium-vms`

> Este documento existe para permitir que um novo chat continue o desenvolvimento sem depender do histórico da conversa anterior.

---

## 1. Diretriz principal do projeto

Princípio de engenharia:

> **Arquitetura suficiente para o problema real.**

Evitar overengineering. O legado r25 funciona como **oracle comportamental**, não como arquitetura a ser copiada integralmente.

O C++ existe para elevar teto de:

- desempenho;
- escalabilidade;
- concorrência;
- memória;
- GPU;
- baixa latência;
- estabilidade.

Nunca declarar PASS/build/performance sem evidência real.

Nunca fazer merge em `main` nem marcar `Human Review Approved` sem aprovação explícita do usuário.

---

## 2. Identidade do produto

Nome oficial:

**SENTRIUM VMS — Intelligent Video Management System**

Namespace C++:

`sentrium::*`

Nome antigo “XC Video IA” identifica somente o legado.

---

## 3. Repositório / baseline

Repositório privado:

`servicosxc-xc/sentrium-vms`

Remote:

`https://github.com/servicosxc-xc/sentrium-vms.git`

Local:

`C:\Projetos\SENTRIUM VMS\sentrium-vms`

Baseline de `main` após F003:

`9a9f67760fa31b09a6cd33f51a7355f3c1b39f69`

Branch atual:

`foundation/004-media-lab`

---

## 4. Foundations concluídas

### F001 — COMPLETE / PASS / HUMAN REVIEW APPROVED

Foundation de toolchain/runtime mínimo.

Principais commits/estado:

- main baseline F001: `b6d3ccd1b847afab144e48700644da76f0595eb3`
- tag: `foundation-001-approved`
- closure: `F001-CLOSE-001`
- Debug/Release: 19/19 testes por configuração/ciclo
- AC01–26 PASS

Toolchain fixada:

- VS Build Tools 17.14.41
- v143/MSVC 14.44
- Windows SDK 10.0.26100.9457
- CMake 4.4.4
- Ninja 1.13.2
- vcpkg tag 2026.10.16

### F002 — COMPLETE / PASS / HUMAN REVIEW APPROVED / MERGED

Application Bootstrap mínimo em Qt 6.12.

Merge em main:

`0562518f1b5ee02b941d5d6323c8e1d341e127f8`

UI provisória:

- tema escuro;
- árvore de dispositivos vazia;
- mosaico vazio;
- Visualizar/Reproduzir;
- Eventos Ao Vivo colapsável;
- footer/build metadata;
- sem dependência de servidor.

### F003 — COMPLETE / PASS / HUMAN REVIEW APPROVED / MERGED

Device Foundation / Intelbras IP-LAN.

Merge em main:

`9a9f67760fa31b09a6cd33f51a7355f3c1b39f69`

Validação real:

- NVR Intelbras
- `192.168.0.100:7000`
- login LAN
- 18 canais reportados
- connect/disconnect/reconnect
- árvore real
- 27/27 testes
- Release PASS

Subset NetSDK mantido pequeno.

---

## 5. Legado r25 — referência comportamental

Legacy path:

`C:\Projetos\VMS NOVO\XC-VideoIA-v1-Foundation-007-r25`

ZIP SHA256:

`18C90459C5B38611B5569C758E287705273F889E8FB9BAB487926538C4A00D48`

AUDIT-001-R1 SHA256:

`9984882C20659B9ED47DB8EE0D4B971DF6C8F203BACA3E47955E0E062914C273`

Fluxo legado:

### Live
NetSDK RAW_DATA → broker → WebSocket XCVI → Client → PlaySDK → HWND

### Headless / LPR
broker compressed → PlaySDK no-window → I420 → sampling → IA/LPR

O WPF legado não fazia render pixel-a-pixel em WriteableBitmap para o live principal.

Subset NetSDK historicamente usado:

- CLIENT_InitEx
- CLIENT_SetAutoReconnect
- CLIENT_LoginEx2
- CLIENT_GetLastError
- CLIENT_Logout
- CLIENT_RealPlayEx
- CLIENT_SetRealDataCallBackEx2
- CLIENT_StopRealPlayEx
- CLIENT_DHPTZControlEx2
- CLIENT_Cleanup

Não portar wrapper C# inteiro.

---

## 6. F004 — estado atual

Foundation:

**SENTRIUM-CPP-Foundation-004 — Media Lab**

Estado:

**AUTHORIZED / IN IMPLEMENTATION**

Objetivo:

1. provar NetSDK → RAW/DHAV em C++;
2. PlaySDK como oracle;
3. verificar compatibilidade FFmpeg;
4. software decode;
5. D3D11VA;
6. comparar custo antes de escolher pipeline de Live View.

ADR-005 continua **Proposed**.

Nenhum merge / Human Review final ainda.

---

## 7. SDKs locais

NetSDK:

`C:\Temp\IntelbrasSDK\NetSDK64`

PlaySDK:

`C:\Temp\IntelbrasSDK\PlaySDK64`

Qt:

`C:\Qt\6.12.0\msvc2022_64`

FFmpeg:

`C:\Temp\FFmpeg\ffmpeg-9.0.2-essentials_build`

Variáveis:

`SENTRIUM_INTELBRAS_SDK_ROOT`  
`SENTRIUM_PLAYSDK_ROOT`  
`SENTRIUM_QT_ROOT`  
`SENTRIUM_FFMPEG_ROOT`

Preflight:

`scripts/windows/sentrium-preflight.ps1`

---

## 8. F004 — mídia implementada

Target:

`sentrium-media-lab`

Biblioteca:

`sentrium-media`

Fila bounded:

- capacidade explícita;
- drop-oldest;
- sequence;
- steady_clock arrival;
- bytes/pacotes;
- drops;
- HWM;
- depth;
- thread-safe.

Principal/Extra explícitos.

Extra usa enum oficial:

`DH_RType_Realplay_1`

Stop oficial:

`CLIENT_StopRealPlayEx`

PlaySDK headless:

- PLAY_GetFreePort
- PLAY_SetStreamOpenMode
- PLAY_OpenStream
- PLAY_SetDecCBStream
- PLAY_SetDecCallBackEx
- PLAY_Play
- PLAY_InputData
- PLAY_Stop
- PLAY_CloseStream
- PLAY_ReleasePort

---

## 9. Evidências PlaySDK reais

### Principal — 1 stream real

Resultado:

- PASS
- 1280×720
- 15 fps
- ~14.9 decoded fps
- ~1.73 Mb/s
- zero queue drops
- zero retries
- zero input failures
- HWM 3
- capture: 2,159,842 bytes

Arquivo local:

`out\sentrium-debug\principal-ch01.raw`

### Extra — 1 stream

Resultado:

- PASS
- 704×480
- 3 fps
- ~3 decoded fps
- ~0.17 Mb/s
- zero drops/retries/failures

### Multi-stream 4 handles

Foram testados 4 canais simultâneos, Extra e Principal.

Importante:

O NVR usado possui somente 3 câmeras configuradas e atualmente apenas canais 1 e 3 têm vídeo útil; canal 2 parou de funcionar.

Portanto:

- 4 RealPlay concorrentes = comprovado;
- 4 portas PlaySDK concorrentes = comprovado;
- 4 câmeras reais simultâneas = **NÃO comprovado**;
- benchmarks 4/16 reais ficam NOT RUN neste hardware.

Os documentos F004-MEDIA-REAL-003/004 foram qualificados com essa limitação.

---

## 10. Mudança de sequência do roadmap

Decisão tomada com o usuário:

Não insistir agora em benchmark 4/16/32 usando hardware incompleto.

A sequência passa a ser:

1. terminar F004 e decisão de mídia;
2. F005 = conectividade/dispositivos;
3. depois Live View/mosaico e benchmarks grandes com NVRs completos.

Documento:

`docs/foundation-005/SENTRIUM-CPP-Foundation-005-DIRECTION.md`

### F005 proposta

Interface real de dispositivos com:

1. LAN / IP direto
2. Cloud / P2P
3. Registro Automático

Cloud legado a auditar:

`SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll`

Semântica futura:

- azul = P2P direto
- amarelo = Relay
- câmera neutra quando não visualizada
- bolinha verde somente quando aberta/ativa

Registro Automático deve ser auditado no SDK oficial antes de implementar.

F005 ainda é **DIRECTION ONLY — NOT AUTHORIZED FOR IMPLEMENTATION**.

---

## 11. FFmpeg — resultado funcional já obtido

Versão fixada:

**FFmpeg 9.0.2 essentials x64 — Gyan**

ZIP SHA256 validado pelo usuário:

`60F467265B1E312373DBCD92200C2618A74850F98D3D078E94296BB3FA2047BA`

Executáveis:

ffprobe SHA256:

`F0D36ECBBDD3BCFAC3EFA078C96C7271C2E68B3810595552AC3B7F17E9A65C52`

ffmpeg SHA256:

`3256173F3F8BFFD7DF12227C68ADF68025EDB1832273A9530688A7BB1ED8EDEC`

Probe:

`scripts/windows/sentrium-ffmpeg-probe.ps1`

Amostra:

`principal-ch01.raw`

Tamanho:

`2,159,842 bytes`

Resultado real:

- FFprobe exit 0
- DemuxDecision = SUPPORTED
- FormatName = dhav
- FormatLongName = Video DAV
- Codec = H.264
- 1280×720
- yuv420p
- SoftwareDecodeDecision = SUPPORTED
- SoftwareDecodeExitCode = 0
- D3D11VADecision = SUPPORTED
- D3D11VAExitCode = 0

Conclusão:

`NetSDK RAW/DHAV → FFmpeg DHAV demux → H.264 decode`

funciona diretamente para a amostra testada.

Nenhum parser customizado foi necessário.

Documento:

`docs/foundation-004/F004-MEDIA-FFMPEG-001.md`

Isso suporta:

- F004-AC-09 PASS
- F004-AC-10 PASS
- F004-AC-11 PASS

Ainda **não prova vantagem de performance**, zero-copy ou seleção de backend.

---

## 12. PONTO EXATO ONDE A CONVERSA PAROU

Foi criado:

`scripts/windows/sentrium-ffmpeg-benchmark.ps1`

Objetivo:

comparar **FFmpeg Software vs D3D11VA** sobre a mesma amostra, com:

- warm-up;
- 3 repetições;
- loop da amostra;
- wall time;
- CPU process time;
- CPU normalizada;
- peak working set;
- frames;
- frames/s.

O usuário tinha localmente commit:

`5a089619ac6f5415311fc88e382b7637d30e8e00`

e rodou:

```powershell
& ".\scripts\windows\sentrium-ffmpeg-benchmark.ps1" `
    -InputPath ".\out\sentrium-debug\principal-ch01.raw"
```

Falhou com:

```text
FFMPEG BENCH FAIL: Software warm-up failed:
Error opening input file -map.
```

Causa provável identificada:

A versão em `5a08961` usava um parâmetro PowerShell chamado:

`$Input`

PowerShell é case-insensitive e `$input` é variável automática/reservada de pipeline, podendo destruir a passagem correta do caminho e fazer `-map` virar o argumento de `-i`.

A branch remota já contém uma alteração ainda **não validada pelo usuário** renomeando:

- parâmetro da função para `$InputFile`
- variável resolvida para `$inputFilePath`

Quando este handoff foi criado, a branch remota estava **2 commits à frente de 5a08961**, modificando esse benchmark.

Portanto o primeiro passo do novo chat deve ser:

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only
```

Depois verificar o script atual e repetir o benchmark.

**Não declarar o benchmark corrigido PASS antes do usuário executar.**

---

## 13. Comando esperado para retomar

No PowerShell onde FFmpeg esteja configurado:

```powershell
$env:SENTRIUM_FFMPEG_ROOT = "C:\Temp\FFmpeg\ffmpeg-9.0.2-essentials_build"

cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only

& ".\scripts\windows\sentrium-ffmpeg-benchmark.ps1" `
    -InputPath ".\out\sentrium-debug\principal-ch01.raw"
```

Se ainda falhar, corrigir o script, não o FFmpeg/probe, pois a compatibilidade funcional já foi comprovada.

---

## 14. O que ainda falta para fechar F004

Antes de `READY FOR HUMAN REVIEW`:

- benchmark controlado Software vs D3D11VA;
- métricas de processo confiáveis;
- implicação de render;
- Release build;
- regressão;
- zero processo residual;
- consolidar cenários NOT RUN por limitação de hardware;
- recomendação final PlaySDK vs FFmpeg SW vs D3D11VA/combinação;
- atualizar ADR-005 apenas após evidência;
- Human Review explícito do usuário antes de merge.

Escala 4/16 com câmeras reais foi adiada por limitação do NVR atual.

---

## 15. Direção de produto depois da F004

F005 deve priorizar:

- interface operacional de dispositivos;
- LAN;
- Cloud/P2P;
- Registro Automático;
- árvore real;
- status de transporte;
- connect/disconnect/remove;
- diagnóstico sanitizado.

Depois disso, usar NVRs completos 16/32 canais para:

- Live View;
- mosaico;
- Principal/Extra;
- benchmarks reais;
- CPU/GPU/memória;
- P2P direto vs Relay;
- reconexão.

---

## 16. Regras permanentes de desenvolvimento

- Não inventar API do SDK.
- Confirmar nomes nos headers/binários reais.
- Não portar wrapper inteiro sem necessidade.
- Não criar framework genérico de plugins sem consumidor real.
- Não guardar credenciais em logs/evidências.
- Não afirmar zero-copy sem prova.
- Não afirmar aceleração GPU apenas porque D3D11VA inicializou.
- Separar ingest/decode/render.
- Manter PlaySDK como oracle até decisão fundamentada.
- Resultado negativo é evidência válida.
- Não mergear `main` sem autorização explícita.
- Não marcar Human Review Approved sem autorização explícita.

---

## 17. Preferência de colaboração do usuário

Responder em português.

Estilo:

- técnico;
- direto;
- implementação-first;
- comandos PowerShell passo a passo;
- analisar logs reais;
- não repetir perguntas já respondidas;
- não exagerar arquitetura;
- avançar decisivamente quando a evidência permitir.

---

## 18. Instrução para o próximo chat

Ao receber este handoff:

1. trate o conteúdo como baseline factual do projeto;
2. consulte o GitHub `servicosxc-xc/sentrium-vms`;
3. leia os documentos F004/F005 relevantes;
4. verifique o HEAD atual da branch `foundation/004-media-lab`;
5. continue exatamente do benchmark FFmpeg;
6. não recomece F001–F003;
7. não faça merge;
8. não peça ao usuário informações já presentes neste documento.

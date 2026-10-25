# SENTRIUM-CPP-Foundation-004-SCOPE 0.1 — MEDIA LAB

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** SENTRIUM-CPP-Foundation-004  
**Branch:** `foundation/004-media-lab`  
**Estado:** **PROPOSED — NOT YET AUTHORIZED FOR IMPLEMENTATION**  
**Princípio:** medir antes de escolher a pipeline definitiva.

---

## 1. Objetivo

Construir um laboratório de mídia C++ controlado para responder, com vídeo real do mesmo equipamento Intelbras já validado na F003:

1. o caminho NetSDK → RAW/DHAV funciona de forma estável em C++;
2. o PlaySDK nativo reproduz/decode o stream corretamente e serve como oracle comportamental;
3. o payload recebido pelo NetSDK é ou não compatível com FFmpeg;
4. caso FFmpeg seja compatível, comparar software decode e D3D11VA;
5. separar custo de ingest, decode e render;
6. produzir evidência suficiente para escolher a pipeline da Foundation-005 Live View.

A F004 é laboratório. Ela **não transforma nenhuma opção em pipeline de produção por teoria**.

---

## 2. Pergunta de decisão

Ao final da F004 devemos conseguir responder:

> Para o SENTRIUM VMS no Windows, qual caminho de mídia deve seguir para o primeiro Live View real: PlaySDK, FFmpeg software, FFmpeg+D3D11VA ou uma combinação/fallback?

É permitido que a resposta seja:

- PlaySDK permanece necessário;
- FFmpeg é viável;
- FFmpeg é parcialmente viável;
- D3D11VA não é viável no cenário testado;
- dados insuficientes / BLOCKED.

O laboratório não deve forçar uma conclusão positiva.

---

## 3. Baseline

Partir da `main` após Foundation-003:

- C++20;
- MSVC/v143 x64;
- CMake + Ninja;
- Qt 6.12.0 permanece disponível, mas não é obrigatório para todos os probes;
- NetSDK externo já validado;
- equipamento Intelbras real por LAN já validado;
- Principal e Extra permanecem streams independentes.

Cloud/P2P continua fora desta Foundation para isolar mídia.

---

## 4. Executável de laboratório

Criar alvo separado, por exemplo:

`sentrium-media-lab`

Não acoplar o experimento diretamente ao `sentrium-client`.

O lab deve aceitar configuração explícita de:

- host;
- porta;
- usuário;
- senha;
- canal;
- stream Principal/Extra;
- backend;
- modo headless/render;
- duração;
- quantidade de canais quando cenário multi-stream.

Credenciais somente em memória e nunca registradas em CSV/JSON/log.

---

## 5. Ingest NetSDK

Adicionar somente o subset de mídia necessário:

- `CLIENT_RealPlayEx`;
- `CLIENT_SetRealDataCallBackEx2`;
- `CLIENT_StopRealPlay`.

Requisitos:

- RealPlay headless para captura do stream;
- callback copia apenas o necessário antes de retornar;
- sequence/generation/arrival monotônico;
- fila limitada;
- métricas de bytes/pacotes/drops;
- teardown determinístico;
- stream Principal e Extra selecionáveis;
- nenhum fallback silencioso Extra → Principal.

Não implementar broker genérico de produção nesta Foundation.

---

## 6. PlaySDK — oracle obrigatório

Usar o kit PlaySDK externo correspondente ao mesmo pacote do NetSDK.

Artefatos esperados:

- `dhplay.h`;
- `dhplay.dll`;
- `dhplay.lib`.

Subset mínimo permitido conforme necessidade real do lab:

- `PLAY_GetFreePort`;
- `PLAY_SetStreamOpenMode`;
- `PLAY_OpenStream`;
- `PLAY_InputData`;
- `PLAY_Play`;
- callback de decode documentado no header;
- `PLAY_Stop`;
- `PLAY_CloseStream`;
- `PLAY_ReleasePort`;
- funções de buffer/error somente se tiverem consumidor direto.

Preservar nomes reais dos exports do SDK.

Objetivos:

- primeiro vídeo real em C++;
- decode headless mensurável;
- preview/render nativo mínimo quando necessário;
- métricas de frames de saída e falhas.

Não portar todo wrapper legado.

---

## 7. Captura RAW/DHAV

O lab deve ser capaz de salvar uma amostra sanitizada do stream comprimido recebido pelo callback, sem credenciais e sem dados de configuração do equipamento.

A amostra existe para identificar:

- container/formato;
- codec;
- extradata;
- fragmentação;
- timestamps quando disponíveis;
- compatibilidade com FFmpeg.

**Não assumir que RAW_DATA = H.264 Annex-B.**

---

## 8. FFmpeg — prova, não premissa

### 8.1 Software decode

FFmpeg software é probe obrigatório **se a amostra RAW/DHAV puder ser demuxada de forma correta**.

A integração deve usar versão fixa e externa ao Git, com root explícito, por exemplo:

`SENTRIUM_FFMPEG_ROOT`

Não baixar automaticamente e não adicionar binários FFmpeg ao repositório.

### 8.2 D3D11VA

Executar probe D3D11VA somente depois que:

- o demux/codec estiver identificado;
- software decode funcionar para o mesmo stream.

D3D11VA deve medir superfícies/frames produzidos sem afirmar zero-copy.

### 8.3 Resultado negativo permitido

Se FFmpeg não conseguir consumir corretamente o DHAV/RAW do callback:

- registrar evidência;
- não inventar parser;
- não extrair H.264 por heurística;
- manter PlaySDK como caminho válido;
- F004 pode PASS como laboratório mesmo com FFmpeg rejeitado, desde que a decisão esteja fundamentada.

---

## 9. Render

Separar modos:

### Headless
Mede ingest + decode sem apresentação.

### Render
Mede ingest + decode + apresentação.

O render inicial pode usar janela nativa controlada do laboratório.

Não construir mosaico definitivo do SENTRIUM nesta Foundation.

A integração definitiva com Qt Quick pertence à F005 depois da decisão de mídia.

---

## 10. Cenários de benchmark

Usar o mesmo equipamento e configurações equivalentes entre backends.

Cenários mínimos:

### Extra
- 1 stream;
- 4 streams;
- 16 streams, se o equipamento possuir quantidade suficiente.

### Principal
- 1 stream;
- 4 streams;
- 16 streams quando suportado pelo equipamento e host de teste.

Para cada cenário aplicável:

- warm-up;
- janela de medição fixa;
- pelo menos duas repetições;
- headless;
- render quando backend suportar.

Se um cenário não for possível, registrar `NOT RUN` com motivo; nunca converter em PASS.

---

## 11. Métricas

Coletar no mínimo:

### Ingest
- pacotes;
- bytes;
- taxa agregada;
- drops de fila;
- depth/HWM;
- último pacote/idade.

### Decode
- frames decodificados;
- decode FPS;
- erros/retries;
- drops;
- tempo callback→frame quando mensurável.

### Render
- frames apresentados;
- render FPS;
- frames descartados;
- idade do frame no momento da apresentação quando mensurável.

### Processo
- CPU;
- Working Set;
- Private Bytes;
- handles;
- threads.

### GPU
Quando disponível e confiável:
- engine Video Decode;
- 3D;
- Copy;
- memória dedicada.

GPU é evidência observacional; ausência de contador confiável não deve gerar métrica inventada.

---

## 12. Saída de evidência

Cada execução deve produzir JSON/CSV com:

- BuildId;
- configuração Debug/Release;
- backend;
- stream;
- canais;
- duração;
- resolução/codec/FPS quando conhecidos;
- métricas;
- falhas;
- versão do SDK/FFmpeg quando disponível;
- sem host, usuário ou senha em evidência persistida quando isso não for necessário.

Manter resultados brutos separados das conclusões.

---

## 13. Backpressure

O lab deve provar que não cria backlog ilimitado.

Usar:

- filas bounded;
- contador de drops;
- HWM;
- idade;
- teardown.

Não introduzir ainda o broker compartilhado final de produção.

---

## 14. Testes automatizados

Sem hardware:

- bounded queue;
- drops/HWM;
- packet lifetime;
- stop/cancellation;
- backend lifecycle fake;
- métricas;
- sanitização de credenciais;
- stream Principal/Extra sem fallback;
- regressão F001–F003.

Com hardware:

- NetSDK RealPlay inicia;
- pacotes chegam;
- StopRealPlay fecha;
- PlaySDK recebe/decode;
- reconexão controlada;
- cenário multi-stream;
- zero processo residual.

---

## 15. Fora do escopo

Proibido nesta Foundation:

- Cloud/P2P;
- libt2u;
- Live View definitivo;
- mosaico definitivo;
- gravação;
- playback;
- timeline;
- DAV local/remoto de produto;
- snapshots de produto;
- PTZ;
- áudio de produto;
- IA;
- LPR;
- eventos/regras;
- banco;
- persistência de dispositivos/credenciais;
- installer;
- abstração genérica de plugins/codecs sem consumidor.

---

## 16. Critérios de aceite

**F004-AC-01** — Debug build PASS.  
**F004-AC-02** — Release build PASS.  
**F004-AC-03** — regressão F001–F003 PASS.  
**F004-AC-04** — NetSDK RealPlay real por LAN entrega pacotes.  
**F004-AC-05** — Principal/Extra permanecem distintos.  
**F004-AC-06** — PlaySDK oracle decodifica pelo menos um stream real.  
**F004-AC-07** — teardown RealPlay/PlaySDK determinístico.  
**F004-AC-08** — fila bounded com drops/HWM observáveis.  
**F004-AC-09** — amostra RAW/DHAV capturada e identificada até o nível suportado pela evidência.  
**F004-AC-10** — FFmpeg software recebe decisão SUPPORTED/UNSUPPORTED/BLOCKED com evidência.  
**F004-AC-11** — D3D11VA recebe decisão SUPPORTED/UNSUPPORTED/NOT APPLICABLE/BLOCKED com evidência.  
**F004-AC-12** — benchmarks 1/4/16 são registrados onde aplicáveis, sem inventar cenários não executados.  
**F004-AC-13** — ingest/decode/render têm métricas separadas.  
**F004-AC-14** — CPU/memória/handles/threads registrados.  
**F004-AC-15** — GPU registrada apenas quando observável de forma confiável.  
**F004-AC-16** — nenhuma credencial aparece em evidência persistida.  
**F004-AC-17** — zero processo residual após ciclos.  
**F004-AC-18** — nenhuma alegação de ganho de performance sem medição.  
**F004-AC-19** — nenhuma pipeline é promovida a produção automaticamente.  
**F004-AC-20** — relatório final recomenda ou rejeita candidatos para F005 com base nos dados.

---

## 17. ADRs

- ADR-004 — client-framework: Proposed.
- ADR-005 — media-pipeline: Proposed e principal ADR informado pela F004.
- ADR-010 — interoperability-intelbras: Proposed.

A F004 pode produzir evidência para ADR-005, mas sua promoção para Accepted exige decisão humana posterior.

---

## 18. Resultado permitido

O executor técnico pode declarar somente:

- `FOUNDATION-004 — PASS / READY FOR HUMAN REVIEW`
- `FOUNDATION-004 — FAIL`
- `FOUNDATION-004 — BLOCKED`

Nenhum backend é declarado vencedor sem os resultados do laboratório.

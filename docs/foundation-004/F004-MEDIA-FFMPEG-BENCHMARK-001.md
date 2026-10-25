# F004-MEDIA-FFMPEG-BENCHMARK-001 — Software vs D3D11VA

**Projeto:** SENTRIUM VMS — Intelligent Video Management System  
**Foundation:** 004 — Media Lab  
**Estado:** PASS DE EXECUÇÃO DO BENCHMARK / SELEÇÃO DE PIPELINE PENDENTE  
**Origem da evidência:** saída de PowerShell e tabelas de `result.json` fornecidas pelo operador em 25/10/2026. O JSON bruto continua local; não foi importado nem recalculado no GitHub.  
**Script:** `scripts/windows/sentrium-ffmpeg-benchmark.ps1` no commit `b499d0b4de84ca8068bfb9543e65da3d2be94593`.

## Ambiente e amostra

- FFmpeg: 9.0.2 essentials (Gyan), SHA256 do executável `3256173F3F8BFFD7DF12227C68ADF68025EDB1832273A9530688A7BB1ED8EDEC`.
- Entrada: `principal-ch01.raw`, 2.159.842 bytes, DHAV/H.264, 1280×720, originalmente capturada por NetSDK LAN.
- Benchmark: 19 loops, 20 passagens efetivas, 2.980 frames por execução, 3 repetições por backend mais warm-up, 16 processadores lógicos.
- Fluxo medido: arquivo capturado → FFmpeg → decode → saída nula; **sem renderização de vídeo real**, sem ingestão de rede ao vivo e sem medição de GPU.

## Resultados do operador

| Métrica média | Software | D3D11VA |
|---|---:|---:|
| Execuções / ExitCode | 3/3, 0 | 3/3, 0 |
| Frames por execução | 2.980 | 2.980 |
| Wall time | 1.504,266 ms | 2.399,157 ms |
| CPU time acumulado | 7.406,250 ms | 3.348,958 ms |
| CPU normalizada | 30,777% | 8,725% |
| Peak Working Set amostrado | 72,665 MB | 116,557 MB |
| Throughput calculado pela média wall time | ~1.981 fps | ~1.242 fps |

### Memória por repetição e contagem de amostras

- Software: 72,918 MB / 43 amostras; 72,387 MB / 45; 72,691 MB / 39.
- D3D11VA: 116,445 MB / 65 amostras; 116,676 MB / 67; 116,551 MB / 71.

O script anterior informava `PeakWorkingSetMB=0` depois do encerramento do FFmpeg. A versão nova consultou memória durante o processo e retornou valores não nulos. É **pico observado por amostragem**, não aferição de máximo absoluto de memória. Private Bytes, handles, threads e GPU não foram medidos nesta saída.

## Interpretação limitada

- D3D11VA exigiu aproximadamente **54,8% menos CPU time acumulado** para o mesmo número de frames, mas **59,5% mais wall time** do que software.
- D3D11VA observou aproximadamente **43,9 MB a mais de Peak Working Set por processo**.
- CPU normalizada é calculada a partir de CPU time / wall time / 16 CPUs lógicas; não substituir esta métrica por consumo total de CPU ao longo de uma execução inteira.
- Há avisos de DTS não monotônico na execução anterior com `-stream_loop`; o resumo atual não incluiu `Stderr` por repetição e, portanto, não comprova que esses avisos desapareceram.
- A saída nula não demonstra renderização, zero-copy, escalabilidade multicâmera, latência ao vivo, nem superioridade de pipeline para produção.

## Impacto nos critérios

- Compatibilidade FFmpeg software e D3D11VA previamente demonstrada continua respaldada para a amostra (F004-AC-10/11).
- Este ensaio fornece **evidência adicional de desempenho por arquivo**, não substitui cenários de ingest/decode/render separados ou benchmarks em 4/16 câmeras reais (F004-AC-12/13).
- CPU e pico amostrado de memória são válidos como observações; AC-14 também exige handles/threads e outras evidências.
- Seleção final PlaySDK vs FFmpeg software vs FFmpeg D3D11VA/combinação: **PENDENTE**.
- ADR-005: **Proposed**. Foundation-004: **IN IMPLEMENTATION**, sem Human Review e sem merge em main.

## Pendências para fechar F004

1. Verificar warning count/stderr do `result.json` local e anexar o resultado bruto sanitizado à evidência apropriada.
2. Completar Release build, CTest e regressão F001–F003.
3. Comprovar shutdown e ausência de processos residuais.
4. Consolidar medições aplicáveis e deixar 4/16 câmeras reais como `NOT RUN` pelo limite de topologia já aprovado.
5. Emitir recomendação técnica de pipeline com limitações explícitas para Human Review, sem aceitar ADR-005 automaticamente.

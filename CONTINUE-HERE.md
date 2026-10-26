# SENTRIUM VMS — CONTINUAR SEM PERDER O ESTADO

**Fonte atual do projeto:** [docs/HANDOFF-SENTRIUM-CURRENT.md](docs/HANDOFF-SENTRIUM-CURRENT.md)  
**Branch:** `foundation/005-device-connectivity`  
**PR:** https://github.com/servicosxc-xc/sentrium-vms/pull/4

O repositório GitHub, seu histórico de commits e os arquivos versionados são a fonte de verdade. O texto desta conversa não é o único registro. Ao mudar de chat, **copie esta mensagem**:

> Continue o desenvolvimento do SENTRIUM VMS via GitHub, repo `servicosxc-xc/sentrium-vms`, branch `foundation/005-device-connectivity`. **Antes de responder, leia `CONTINUE-HERE.md` e `docs/HANDOFF-SENTRIUM-CURRENT.md` no próprio GitHub e consulte o commit HEAD mais recente**. Desenvolvimento C++/Qt direto no GitHub, sem Codex, em blocos grandes. AutoRegistro real já conectou um NVR Intelbras com 8 canais. Cloud/P2P tem código nativo de parser/UDP/sinalização, mas falta bootstrap autorizado/túnel real; não usar SIM Next nem terceiro `dh-fwd` bloqueado pelo Defender. Última compilação parou em LNK1168 (SENTRIUM ainda aberto). Para validar há `scripts/windows/sentrium-validate-f005-cloud.ps1`. **Não reinicie etapas, não repita perguntas respondidas, não invente resultados, não faça merge na main; atualize handoff após os avanços.**

## Objetivo imediato

1. Validar o lote atual C++ após fechar o cliente Windows (preflight, CMake, build, D-9/D-10 em um comando).
2. Continuar a implementação **autônoma e segura** do protocolo Cloud Intelbras/Dahua; não afirmar login real sem bootstrap/autorização e teste com gravador.
3. Quando conexão Cloud real existir, acessar NVRs remotos 16/32 canais e só então avançar no mosaico e em benchmarks representativos.
4. Melhoria estética da Home (ícones menores arredondados/nome abaixo) fica **adiada**.

**Não colocar usuários/senhas/seriais Cloud em documentação ou logs.**

O handoff histórico `docs/HANDOFF-SENTRIUM-2026-10-25.md` cobre F004; não é o ponto atual. O **handoff CURRENT** acima foi criado especificamente para evitar perda de progresso entre conversas.

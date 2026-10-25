# SENTRIUM — DataRoot e efeitos colaterais

DataRoot único: `%LOCALAPPDATA%\Sentrium\VMS\dev\Foundation-001`, obtido por API Windows. O relatório `rollback-result.json` registra o caminho concreto apenas na evidência de desenvolvimento; a resposta pública de health não contém caminhos de usuário.

Runtime: `runtime/sentrium.lock`, `runtime/logs/sentrium-{0,1,2}.jsonl`. O lock de arquivo exclusivo impede dois processos de escrever simultaneamente nos mesmos slots. Não é um transporte IPC. Fixtures: `tests/<UUID>`; cada teste gera identidade própria. Os testes apagam apenas sua fixture após os handles saírem de escopo. A árvore `tests` estava vazia ao fim dos testes e do rollback. A execução demonstrativa de health deixou somente seu runtime normal.

A auditoria de fonte dos pontos de escrita/abertura está em `filesystem-process-api-audit.txt`. O produto escreve somente através dos handles verificados; os scripts escrevem builds/temp/evidências no workspace. O código do produto não chama CreateProcess, TerminateProcess, APIs de serviço, alterações de registro ou APIs de rede. CreateProcess e a limpeza forçada em timeout existem apenas no harness de teste; nenhum timeout ocorreu. A inspeção de importações corrobora a ausência de transporte de rede/IPC implementado pela Foundation.

Não houve tracing global de todas as chamadas de filesystem do Windows. A afirmação de confinamento se baseia na revisão dos pontos de escrita do produto, abertura por handle, fixtures negativas e inventário dos artefatos, dentro do threat model aprovado. Efeitos internos do runtime/OS não são uma promessa de ausência absoluta de escrita do sistema.

Nenhuma operação leu ou enumerou um DataRoot legado, alterou serviços, firewall, drivers, SDKs ou variáveis globais. A inspeção do registro foi somente de leitura, para identificação dos pacotes SDK. Os builds herdaram um ambiente oficial configurado somente em processo.

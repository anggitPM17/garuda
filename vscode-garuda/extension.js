const vscode = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let client;

function activate(context) {
    const serverExecutable = vscode.workspace.getConfiguration('garuda').get('languageServer.executable') || 'garuda_lsp_server';

    const serverOptions = {
        command: serverExecutable,
        transport: TransportKind.stdio
    };

    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'garuda' }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.grd')
        }
    };

    client = new LanguageClient('garudaLsp', 'Garuda Language Server', serverOptions, clientOptions);
    client.start();
}

function deactivate() {
    if (!client) {
        return undefined;
    }
    return client.stop();
}

module.exports = {
    activate,
    deactivate
};

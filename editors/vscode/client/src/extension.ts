import * as vscode from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind,
} from 'vscode-languageclient/node';

let client: LanguageClient | undefined;

async function startClient(): Promise<void> {
    const config = vscode.workspace.getConfiguration('cx');
    const serverOptions: ServerOptions = {
        command: config.get<string>('languageServer.path', 'cx-lsp'),
        args: config.get<string[]>('languageServer.args', []),
        transport: TransportKind.stdio,
    };
    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'cx' }],
        initializationOptions: {
            importSearchPaths: config.get<string[]>('languageServer.importSearchPaths', []),
            defines: config.get<string[]>('languageServer.defines', []),
        },
    };
    client = new LanguageClient('cxLanguageServer', 'cx Language Server', serverOptions, clientOptions);
    try {
        await client.start();
    } catch (error) {
        client = undefined;
        vscode.window.showErrorMessage(
            `Couldn't start cx-lsp (${error}). Is it on PATH, or set cx.languageServer.path?`,
        );
    }
}

export async function activate(context: vscode.ExtensionContext): Promise<void> {
    await startClient();

    context.subscriptions.push(
        vscode.commands.registerCommand('cx.restartLanguageServer', async () => {
            if (client) {
                await client.stop();
                client = undefined;
            }
            await startClient();
        }),
    );
}

export function deactivate(): Thenable<void> | undefined {
    return client?.stop();
}

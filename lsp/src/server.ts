import {
    createConnection,
    TextDocuments,
    Diagnostic,
    DiagnosticSeverity,
    ProposedFeatures,
    InitializeParams,
    CompletionItem,
    TextDocumentPositionParams,
    TextDocumentSyncKind,
    InitializeResult,
    MarkupKind
} from 'vscode-languageserver/node';

import { TextDocument } from 'vscode-languageserver-textdocument';
import { GARUDA_KEYWORDS, GARUDA_STDLIB } from './keywords';

const connection = createConnection(ProposedFeatures.all);
const documents: TextDocuments<TextDocument> = new TextDocuments(TextDocument);

connection.onInitialize((params: InitializeParams) => {
    const result: InitializeResult = {
        capabilities: {
            textDocumentSync: TextDocumentSyncKind.Incremental,
            completionProvider: {
                resolveProvider: false,
                triggerCharacters: ['.']
            },
            hoverProvider: true
        }
    };
    return result;
});

documents.onDidChangeContent(change => {
    validateTextDocument(change.document);
});

function validateTextDocument(textDocument: TextDocument): void {
    const text = textDocument.getText();
    const diagnostics: Diagnostic[] = [];

    const stack: { char: string; line: number; col: number }[] = [];
    const lines = text.split('\n');

    let inString = false;
    let stringStartLine = 0;
    let stringStartCol = 0;

    for (let lineIndex = 0; lineIndex < lines.length; lineIndex++) {
        const line = lines[lineIndex];
        if (line.trim().startsWith('#')) continue;

        for (let colIndex = 0; colIndex < line.length; colIndex++) {
            const char = line[colIndex];

            if (char === '"' && (colIndex === 0 || line[colIndex - 1] !== '\\')) {
                if (!inString) {
                    inString = true;
                    stringStartLine = lineIndex;
                    stringStartCol = colIndex;
                } else {
                    inString = false;
                }
                continue;
            }

            if (inString) continue;

            if (char === '(' || char === '[' || char === '{') {
                stack.push({ char, line: lineIndex, col: colIndex });
            } else if (char === ')' || char === ']' || char === '}') {
                if (stack.length === 0) {
                    diagnostics.push({
                        severity: DiagnosticSeverity.Error,
                        range: {
                            start: { line: lineIndex, character: colIndex },
                            end: { line: lineIndex, character: colIndex + 1 }
                        },
                        message: `Unmatched closing bracket '${char}'`,
                        source: 'Garuda LSP'
                    });
                } else {
                    const last = stack.pop()!;
                    const expected = last.char === '(' ? ')' : last.char === '[' ? ']' : '}';
                    if (char !== expected) {
                        diagnostics.push({
                            severity: DiagnosticSeverity.Error,
                            range: {
                                start: { line: lineIndex, character: colIndex },
                                end: { line: lineIndex, character: colIndex + 1 }
                            },
                            message: `Mismatched bracket '${char}', expected '${expected}' for '${last.char}' at line ${last.line + 1}`,
                            source: 'Garuda LSP'
                        });
                    }
                }
            }
        }
    }

    if (inString) {
        diagnostics.push({
            severity: DiagnosticSeverity.Error,
            range: {
                start: { line: stringStartLine, character: stringStartCol },
                end: { line: stringStartLine, character: stringStartCol + 1 }
            },
            message: 'Unterminated string literal',
            source: 'Garuda LSP'
        });
    }

    for (const unclosed of stack) {
        diagnostics.push({
            severity: DiagnosticSeverity.Error,
            range: {
                start: { line: unclosed.line, character: unclosed.col },
                end: { line: unclosed.line, character: unclosed.col + 1 }
            },
            message: `Unclosed bracket '${unclosed.char}'`,
            source: 'Garuda LSP'
        });
    }

    connection.sendDiagnostics({ uri: textDocument.uri, diagnostics });
}

connection.onCompletion(
    (_textDocumentPosition: TextDocumentPositionParams): CompletionItem[] => {
        const doc = documents.get(_textDocumentPosition.textDocument.uri);
        if (!doc) return [];

        const items: CompletionItem[] = [];

        for (const kw of GARUDA_KEYWORDS) {
            items.push({
                label: kw.label,
                kind: kw.kind,
                documentation: {
                    kind: MarkupKind.Markdown,
                    value: kw.documentation
                }
            });
        }

        for (const fn of GARUDA_STDLIB) {
            items.push({
                label: fn.label,
                kind: fn.kind,
                detail: fn.detail,
                documentation: {
                    kind: MarkupKind.Markdown,
                    value: fn.documentation
                }
            });
        }

        return items;
    }
);

connection.onHover((params) => {
    const doc = documents.get(params.textDocument.uri);
    if (!doc) return null;

    const offset = doc.offsetAt(params.position);
    const text = doc.getText();

    let left = offset;
    while (left > 0 && /[a-zA-Z0-9_\.]/.test(text[left - 1])) left--;
    let right = offset;
    while (right < text.length && /[a-zA-Z0-9_\.]/.test(text[right])) right++;

    const word = text.slice(left, right);
    if (!word) return null;

    const kw = GARUDA_KEYWORDS.find(k => k.label === word);
    if (kw) {
        return {
            contents: {
                kind: MarkupKind.Markdown,
                value: `**Garuda Keyword**: \`${kw.label}\`  \n${kw.documentation}`
            }
        };
    }

    const fn = GARUDA_STDLIB.find(f => f.label === word);
    if (fn) {
        return {
            contents: {
                kind: MarkupKind.Markdown,
                value: `\`${fn.detail}\`  \n\n${fn.documentation}`
            }
        };
    }

    return null;
});

documents.listen(connection);
connection.listen();

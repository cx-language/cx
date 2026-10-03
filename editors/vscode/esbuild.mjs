import * as esbuild from 'esbuild';
import * as fs from 'node:fs';

// Drop stale outputs (e.g. a dev sourcemap must not ship in a production bundle).
fs.rmSync('client/out', { recursive: true, force: true });

const production = process.argv.includes('--production');
const watch = process.argv.includes('--watch');

const ctx = await esbuild.context({
    entryPoints: ['client/src/extension.ts'],
    bundle: true,
    outfile: 'client/out/extension.js',
    external: ['vscode'],
    format: 'cjs',
    platform: 'node',
    target: 'node20',
    sourcemap: !production,
    minify: production,
});

if (watch) {
    await ctx.watch();
} else {
    await ctx.rebuild();
    await ctx.dispose();
}

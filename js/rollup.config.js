import filesize from 'rollup-plugin-filesize';
import resolve from '@rollup/plugin-node-resolve';

const escapeNullByte = {
  name: 'escapeNullByte',
  transform(code) {
    code = code.replaceAll('\0', '\\0');
    return {
      code,
    };
  },
};

export default {
  input: 'index.js',
  output: {
    file: 'building-habits.bundled.js',
    format: 'esm',
    minifyInternalExports: false,
  },
  onwarn(warning) {
    if (warning.code !== 'CIRCULAR_DEPENDENCY') {
      console.error(`(!) ${warning.message}`);
    }
  },
  plugins: [
    resolve(),
    escapeNullByte,
    filesize({
      showBrotliSize: true,
    })
  ]
}

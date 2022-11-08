import { createVar, globalStyle } from '@vanilla-extract/css';

export const multiplier = createVar();

globalStyle('*', {
  boxSizing: 'border-box',
});

globalStyle(':root', {
  fontFamily: 'Helvetica, Arial, sans-serif',
  fontSize: 16,
  lineHeight: 24,
  fontWeight: 400,
  margin: 0,
  padding: 0,
  backgroundColor: '#9b8d82',
  fontSynthesis: 'none',
  textRendering: 'optimizeLegibility',
  height: '100%',
  width: '100%',
  vars: {
    [multiplier]: '1.5',
  },
});

globalStyle('html, body, #app', {
  height: '100%',
  width: '100%',
  margin: 0,
  padding: 0,
  '@media': {
    '(max-width: 470px)': {
      padding: 0,
      margin: 0,
    },
  },
});

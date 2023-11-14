import { createVar, globalStyle, style } from '@vanilla-extract/css';

export const multiplier = createVar();

globalStyle('*', {
  boxSizing: 'border-box',
});

globalStyle(':root', {
  fontFamily: 'Helvetica, Arial, sans-serif',
  fontSize: 16,
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

globalStyle('select', {
  padding: '10px 10px',
  fontSize: '1em',
});

export const connectionInformation = style({
  textAlign: 'center',
  fontSize: '2em',
  color: '#fff',
  textTransform: 'uppercase',
  height: '100%',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
});

export const toastNotificationStyle = style({
  position: 'fixed',
  left: '15px',
  top: '15px',
  padding: '8px 12px',
  backgroundColor: '#fff',
  borderRadius: '0',
  boxShadow: 'rgb(0 0 0 / 20%) 0 15px 15px',
  zIndex: 99,
  color: '#000'
});

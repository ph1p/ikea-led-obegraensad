import { style } from '@vanilla-extract/css';
import { multiplier } from './main.css';

export const wrapper = style({
  display: 'flex',
  height: '100%',
  width: '100%',
  flexDirection: 'column',
  justifyContent: 'center',
  '@media': {
    '(max-width: 470px)': {
      padding: 0,
      margin: 0,
    },
  },
});

export const ledScreen = style({
  position: 'relative',
  margin: '0 auto',
  width: `calc(360px * ${multiplier})`,
  height: `calc(520px * ${multiplier})`,
  backgroundColor: '#111',
  border: `calc(14px * ${multiplier}) solid #000`,
  boxSizing: 'border-box',
  selectors: {
    '&.disabled': {
      pointerEvents: 'none',
      opacity: 0.3,
    },
  },
  '@media': {
    '(max-width: 470px)': {
      position: 'relative',
      margin: '0 auto',
      width: '100vw',
      height: `calc(
      (calc(360px * ${multiplier}) / calc(520px * ${multiplier})) *
        100vh
    )`,
      backgroundColor: '#111',
      border: `calc(14px * ${multiplier}) solid #000`,
      boxSizing: 'border-box',
      aspectRatio: 3 / 4,
    },
  },
});

export const cable = style({
  position: 'absolute',
  bottom: '-47vh',
  left: -60,
  width: '40px',
  height: '50vh',
  borderWidth: ' 6px 0 0 6px',
  borderRadius: '30px 0 0 0',
  borderStyle: 'solid',
  borderColor: '#000',
  boxSizing: 'border-box',
  '@media': {
    '(max-width: 470px)': {
      display: 'none',
    },
  },
});

export const grid = style({
  display: 'grid',
  gridTemplateRows: 'repeat(16, 1fr)',
  gridTemplateColumns: 'repeat(16, 1fr)',
  width: '100%',
  height: '100%',
  padding: 10,
});

export const ledWrapper = style({
  padding: `calc(3px * ${multiplier})`,
  display: 'flex',
});

export const ledInner = style({
  alignSelf: 'center',
  width: '100%',
  backgroundColor: '#222',
  position: 'relative',
  transition: 'all 0.2s',
  selectors: {
    '&.active': {
      backgroundColor: '#fff',
    },
    '&::after': {
      content: '',
      paddingTop: '100%',
      display: 'block',
    },
  },
});

export const controls = style({
  position: 'relative',
  justifyContent: 'center',
  width: `calc(360px * ${multiplier})`,
  margin: '40px auto',
  gridTemplateColumns: 'repeat(3, auto)',
});

export const controlRow = style({
  marginTop: 40,
  gap: 15,
  display: 'flex',
  flexWrap: 'wrap',
});

export const button = style({
  color: '#fff',
  backgroundColor: '#0158ab',
  border: 0,
  padding: '10px 15px',
  textTransform: 'uppercase',
  fontSize: 16,
  lineHeight: '24px',
  letterSpacing: 1,
  cursor: 'pointer',
  fontWeight: 'bold',
  selectors: {
    '&:active': {
      transform: 'translateY(-3px)',
    },
    '&:hover': {
      opacity: 0.7,
    },
  },
});

export const connectionInformation = style({
  textAlign: 'center',
  fontSize: '2em',
});

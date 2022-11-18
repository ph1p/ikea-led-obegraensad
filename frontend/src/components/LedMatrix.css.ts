import { style } from '@vanilla-extract/css';
import { multiplier } from '../main.css';

export const ledScreen = style({
  position: 'relative',
  scrollSnapAlign: 'center',
  margin: '0 auto',
  flex: '0 0 auto',
  display: 'inline-block',
  width: `calc(360px * ${multiplier})`,
  height: `calc(520px * ${multiplier})`,
  backgroundColor: '#111',
  border: `calc(14px * ${multiplier}) solid #000`,
  opacity: 0.5,
  transition: 'opacity .3s',
  selectors: {
    '&.disabled': {
      pointerEvents: 'none',
      opacity: '0.3 !important',
    },
    '&.in-view': {
      opacity: 1
    }
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
      aspectRatio: 3 / 4,
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

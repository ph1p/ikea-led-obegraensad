import { style } from '@vanilla-extract/css';
import { multiplier } from './index.css';

export const wrapper = style({
  height: '100%',
  position: 'relative',
});

export const screensWrapper = style({
  scrollSnapType: 'x mandatory',
  display: 'flex',
  flexFlow: 'row nowrap',
  WebkitOverflowScrolling: 'touch',
  overflowX: 'auto',
  gap: 35,
  height: '100%',
  width: '100vw',
  alignItems: 'center',
  padding: `0 calc(100vw - (360px * ${multiplier}) - 50vw + ((360px/2) * ${multiplier}))`,
});

export const controls = style({
  position: 'fixed',
  left: 0,
  bottom: 30,
  display: 'flex',
  gap: 15,
  width: '100%',
  justifyContent: 'center',
});

export const emptyScreen = style({
  textAlign: 'center',
  fontSize: '2em',
  color: '#fff',
  height: '100%',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
});

export const screenHeader = style({
  display: 'flex',
  justifyContent: 'space-between'
});

export const screenCount = style({
  textAlign: 'center',
  fontSize: '2em',
  color: '#fff',
  height: '100%',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
});

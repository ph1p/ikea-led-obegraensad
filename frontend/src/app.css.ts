import { style } from '@vanilla-extract/css';
import { multiplier } from './main.css';

export const wrapper = style({
  display: 'grid',
  padding: '35px 20px',
  height: '100%',
  justifyContent: 'center',
  alignItems: 'center',
  '@media': {
    '(max-width: 470px)': {
      padding: '15px 0',
      margin: 0,
    },
  },
});

export const controlColumn = style({
  gap: 10,
  display: 'flex',
});

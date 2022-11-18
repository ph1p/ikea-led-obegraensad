import { style } from '@vanilla-extract/css';

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

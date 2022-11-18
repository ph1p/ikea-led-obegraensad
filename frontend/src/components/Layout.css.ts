import { globalStyle, style } from '@vanilla-extract/css';

export const content = style({});

export const footer = style({
  backgroundColor: '#fff',
  display: 'flex',
  position: 'relative',
  justifyContent: 'center',
  padding: '0 25px',
  gap: 30,
  alignItems: 'center',
  height: '100%',
  fontSize: '1.2em',
  color: '#000',
});

globalStyle(`${footer} a`, {
  display: 'inline-block',
  textUnderlinePosition: 'under',
  textDecorationThickness: '0.2em',
  color: '#000',
  marginLeft: 40,
  alignSelf: 'center',
  textTransform: 'uppercase',
});

export const layout = style({
  display: 'grid',
  gridTemplateRows: '1fr 100px',
  height: '100%',
});

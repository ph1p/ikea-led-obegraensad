export const loadImageAndGetDataArray = (cb: (data: number[]) => void) => {
  const matrixSize = 16;
  const tempFileInput = document.createElement('input');
  tempFileInput.type = 'file';
  tempFileInput.click();

  tempFileInput.onchange = (e: Event) => {
    if (!e.target) return;
    const file = (e.target as HTMLInputElement).files?.[0]!;
    const reader = new FileReader();
    reader.readAsDataURL(file);
    reader.onload = () => {
      const image = new Image(matrixSize, matrixSize);
      image.src = reader.result as string;
      image.onload = () => {
        const canvas = new OffscreenCanvas(matrixSize, matrixSize);
        const ctx = canvas.getContext('2d');

        ctx?.drawImage(image, 0, 0, matrixSize, matrixSize);
        const imgData = ctx?.getImageData(0, 0, matrixSize, matrixSize);
        if (imgData) {
          const data: number[] = [];
          for (var i = 0; i < imgData.data.length; i += 4) {
            const isActive =
              imgData.data[i] + imgData.data[i + 1] + imgData.data[i + 2] <= 383
                ? 1
                : 0;
            data.push(isActive);
          }

          cb(data);
        }
      };
    };
  };
};

export const rotateArrayByDegree = (data: number[], degree: number) => {
  const numRotations = degree / 90;
  const isRight = numRotations > 0;

  [...new Array(Math.abs(numRotations))].forEach(() => {
    data = rotateArray(data, isRight);
  });

  return data;
};

export const rotateArray = (matrix: number[], turnRight = false) => {
  const SIZE = 16;
  const newState = [...matrix];
  for (let i = 0; i < SIZE; i++) {
    for (let j = i; j < SIZE; j++) {
      let temp = newState[i * SIZE + j];
      newState[i * SIZE + j] = newState[j * SIZE + i];
      newState[j * SIZE + i] = temp;
    }
  }

  for (let i = 0; i < SIZE; i++) {
    let col1 = 0;
    let col2 = SIZE - 1;
    while (col1 < col2) {
      let index1 = i * SIZE + col1;
      let index2 = i * SIZE + col2;
      if (!turnRight) {
        index1 = col1 * SIZE + i;
        index2 = col2 * SIZE + i;
      }

      let temp = newState[index1];
      newState[index1] = newState[index2];
      newState[index2] = temp;
      col1++;
      col2--;
    }
  }
  return newState;
};

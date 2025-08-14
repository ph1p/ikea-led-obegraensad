export const loadImageAndGetDataArray = (cb: (data: number[]) => void) => {
  const matrixSize = 16;
  const tempFileInput = document.createElement("input");
  tempFileInput.type = "file";
  tempFileInput.click();

  tempFileInput.onchange = (e: Event) => {
    if (!e.target) return;
    const file = (e.target as HTMLInputElement).files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.readAsDataURL(file);
    reader.onload = () => {
      const image = new Image(matrixSize, matrixSize);
      image.src = reader.result as string;
      image.onload = () => {
        const canvas = new OffscreenCanvas(matrixSize, matrixSize);
        const ctx = canvas.getContext("2d");

        ctx?.drawImage(image, 0, 0, matrixSize, matrixSize);
        const imgData = ctx?.getImageData(0, 0, matrixSize, matrixSize);
        if (imgData) {
          const data: number[] = [];
          for (let i = 0; i < imgData.data.length; i += 4) {
            const isActive =
              imgData.data[i] + imgData.data[i + 1] + imgData.data[i + 2] <= 383 ? 1 : 0;
            data.push(isActive);
          }

          cb(data);
        }
      };
    };
  };
};

export const rotateArray = (matrix: number[], rotations: number) => {
  const SIZE = 16;
  const newState = [...matrix];

  const swap = (arr: number[], i: number, j: number) => {
    const temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  };

  for (let row = 0; row < SIZE / 2; row++) {
    for (let col = row; col < SIZE - row - 1; col++) {
      for (let r = 0; r < rotations; r++) {
        swap(newState, row * SIZE + col, col * SIZE + (SIZE - 1 - row));
        swap(newState, row * SIZE + col, (SIZE - 1 - row) * SIZE + (SIZE - 1 - col));
        swap(newState, row * SIZE + col, (SIZE - 1 - col) * SIZE + row);
      }
    }
  }

  return newState;
};

export const chunkArray = (input: number[], size: number) =>
  input.reduce((resultArray: number[][], item, index) => {
    const cIndex = Math.floor(index / size);

    if (!resultArray[cIndex]) {
      resultArray[cIndex] = [];
    }

    resultArray[cIndex].push(item);

    return resultArray;
  }, []);

export const matrixToHexArray = (matrix: number[]) =>
  chunkArray(matrix, 8).map((chunk) => parseInt(parseInt(chunk.join(""), 2).toString(), 10));

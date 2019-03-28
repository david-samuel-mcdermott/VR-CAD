const getHandContour = (handMask) => {
  const contours = handMask.findContours(
    cv.RETR_EXTERNAL,
    cv.CHAIN_APPROX_SIMPLE
  );
  // largest contour
  return contours.sort((c0, c1) => c1.area - c0.area)[0];
};

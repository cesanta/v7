Array.prototype.indexOf = function(a, b) {
  if (!b || b < 0) b = 0;
  for (var i = b; i < this.length; i++) {
    if (this[i] == a) {
      return i;
    }
  }
  return -1;
}

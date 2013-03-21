%   W_next = negent_tanh( W, Z )
%
%   Applies the ICA learning rule:
%     w_next = E{z * g(w' * z)} - E{g`(w' * z)} * w
%   where E{.} is the expected value, and g`(.) is the first derivate of g(.).
%
%   This function sets
%     g(x) = x^3
%
%   With 'W' equal to the matrix of (row) vectors and Z equal to the whitened
%   set of observation data (rows as variables and columns as observations),
%   this function applies one iteration of the above learning rule and returns
%   the approximated next guess for the matrix W as W_next. If W_next and W are
%   within a certain epsilon (ignoring sign), then the matrices are said to have
%   converged.
%
function W_next = negent_cube( W, Z )
  W_next = ((W*Z).^3 * Z' - diag(sum((3 * (W*Z).^2)')) * W) / length(Z);
end

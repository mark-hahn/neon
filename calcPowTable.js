const EXP_SIZE = 32;

let code = 
`\n#define EXP_SIZE ${EXP_SIZE+1}\n
const u16 linToExp[EXP_SIZE] = {0x0000,`;

let val = 1; oldBinVal = 0;
console.log((1-1).toString().padStart(6));
for(let idx = 0; idx < EXP_SIZE-1; idx++) {
  const newVal      = val * Math.sqrt(2);
  const NewValRound = Math.round(newVal);
  const newValHex   = NewValRound.toString(16).padStart(4);
  const newValStr   = NewValRound.toString().padStart(6);
  const binValStr   = NewValRound.toString(2);
  const newBinVal   = parseInt(binValStr,2);
  const binIncPc    = (' '+Math.round((newBinVal/oldBinVal - 1)*100))
                      .toString().padStart(4);
              // let x = 0.toString();
  console.log(newVal, newValStr, binIncPc+'%');
  code += '0x'+ NewValRound.toString(16).padStart(4,0) + ', ';
  val       = newVal;
  oldBinVal = newBinVal;
}
console.log(code + '0xffff};');

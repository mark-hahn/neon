const jscad                   = require('@jscad/modeling');
const {union, subtract}       = jscad.booleans;
const {sphere, cube, cuboid, cylinderElliptic, cylinder}  
                              = jscad.primitives;
const {vectorChar}            = jscad.text;
const {hull, hullChain}       = jscad.hulls;
const {translate, translateZ} = jscad.transforms;

const debug      = true;
const debugScale = true;

// ------ default params --------- 
let fontIdx     = 0;
let text        = 'W';
let fontsizeAdj = 1.1;
let vertOfs     = -7;
let genHulls    = true;
let genHoles    = true;
let genPlate    = true;

// channel mm, 0.2 is expansion
const radius = (debugScale? .2 : 0.75 + 0.2); 

const stepDist   = 0.1;        // mm step size when backing up
const segments   = 16;         // sphere segments
const bkupDist   = 3.0;        // dist to back up, frac times radius
const holeTop    = 1.5*radius; // only affects top sphere
const holeBot    = 2.0*radius; // only affects bottom sphere

const plateW      = 182.3;
const plateH      = 84.4;
const plateDepth  = 2.4;
const filetRadius = 2;
const textZofs    = 0.75;  // fraction of diameter below the surface
const padSides    = 20;
const padTopBot   = 15;
// const baseline   = 0.3;   // fraction of plateH

const ptEq         = (A,B) => A && B && A[0] == B[0] && A[1] == B[1];
const vecEq        = (A,B) => A && B && ptEq(A[0],B[0]) && ptEq(A[1],B[1]);
const vecRevEq     = (A,B) => A && B && ptEq(A[0],B[1]) && ptEq(A[1],B[0]);
const ptTouchesEnd = (P,V) => P && V && ptEq(P, V[0]) || ptEq(P, V[1]);

// return intersection point if exists
// but only if point isn't one of CD end points
// https://algs4.cs.princeton.edu/91primitives
const intersectionPoint = (segAB, segCD) => {
  const [[Ax,Ay], [Bx,By]] = segAB;
  const [[Cx,Cy], [Dx,Dy]] = segCD;
  const numeratorR  = ((Ay-Cy)*(Dx-Cx) - (Ax-Cx)*(Dy-Cy));
  const numeratorS  = ((Ay-Cy)*(Bx-Ax) - (Ax-Cx)*(By-Ay));
  const denominator = ((Bx-Ax)*(Dy-Cy) - (By-Ay)*(Dx-Cx));
  if(denominator == 0) return null; // parallel
  const r = numeratorR / denominator;
  const s = numeratorS / denominator;
  if(!(r >= 0 && r <= 1 && s >= 0 && s <= 1)) 
    return null;   // doesn't intersect
  const iPnt = [Ax + r*(Bx-Ax), Ay + r*(By-Ay)]; // intersection point
  if(ptTouchesEnd(iPnt,segAB) || ptTouchesEnd(iPnt,segCD)) 
    return null;  // ends of vectors at same point
  return iPnt;
}

// vector math
// http://www.fundza.com/vectors/vectors.html
const dot = (v,w) => {
  const [x,y,z] = v;
  const [X,Y,Z] = w;
  return x*X + y*Y + z*Z;
}
const length = (v) => {
  const[x,y,z] = v;
  return Math.sqrt(x*x + y*y + z*z);
}
const vector = (b,e) => {
  const[x,y,z] = b;
  const[X,Y,Z] = e;
  return [X-x, Y-y, Z-z];
}
const unit = (v) => {
  const [x,y,z] = v;
  const mag = length(v);
  return [x/mag, y/mag, z/mag];
}
const distance = (p0,p1) => {
  return length(vector(p0,p1));
}
const scale = (v,sc) => {
  const [x,y,z] = v;
  return [x * sc, y * sc, z * sc];
}
const add = (v,w) => {
  const [x,y,z] = v;
  const [X,Y,Z] = w;
  return [x+X, y+Y, z+Z];
}

// http://www.fundza.com/vectors/point2line/index.html
//get the distance between a point and a line
const distPntToVec = (pnt, vec) => {
  pnt                  = [pnt[0], 0, pnt[1]];
  let [start, end]     = vec;
  start                = [start[0], 0, start[1]];
  end                  = [end[0],   0, end[1]];
  const line_vec       = vector(start, end);
  const pnt_vec        = vector(start, pnt);
  const line_len       = length(line_vec);
  const line_unitvec   = unit(line_vec);
  const pnt_vec_scaled = scale(pnt_vec, 1.0/line_len);
  const dt             = dot(line_unitvec, pnt_vec_scaled);
  const t              = Math.max(Math.min(dt,1),0);
  let   nearest        = scale(line_vec, t);
  const dist           = distance(nearest, pnt_vec);
  return dist;
}

const showVec = (pfx, vec) => {
  // console.log('vec[0][0]', vec[0][0]);
  // console.log('vec[0][1]', vec[0][1]);
  // console.log('vec[1][0]', vec[1][0]);
  // console.log('vec[1][1]', vec[1][1]);
  console.log( pfx + ' ' +
    ((typeof vec?.[0]?.[0] === 'undefined' || 
      typeof vec?.[0]?.[1] === 'undefined' ||
      typeof vec?.[1]?.[0] === 'undefined' ||
      typeof vec?.[1]?.[1] === 'undefined') ? 'null' : (
      vec[0][0].toFixed(1).padStart(4) + ','    + 
      vec[0][1].toFixed(1).padStart(4) + ' -> ' +
      vec[1][0].toFixed(1).padStart(4) + ','    + 
      vec[1][1].toFixed(1).padStart(4)))
  );
}

// return point on vec far enough away from prevVec
const backUpPoint = (prevVec, vec, chkHead) => {
  // vec is A -> B and scan is A -> B
  const [[Ax, Ay],[Bx, By]] = vec;
  if(debug)  showVec('enter backUpPoint, prevVec', prevVec);
  if(debug)  showVec('                   vec    ', vec);
  const vecW   = Bx-Ax;
  const vecH   = By-Ay;
  const vecLen = Math.sqrt((vecW*vecW)+(vecH*vecH));
  // walk vec, each stepDist
  for(let distOnVec = 0; distOnVec < vecLen; 
                         distOnVec += stepDist) {      
    const frac = distOnVec/vecLen;
    const trialPoint = 
      (chkHead ? [Bx-frac*vecW, By-frac*vecH]
               : [Ax+frac*vecW, Ay+frac*vecH]);
    const dist2prev  = distPntToVec(trialPoint, prevVec);
    if(dist2prev > bkupDist * radius) return trialPoint;
  }
  // vec was shorter then stepDist
  return null;
}

let lastVec = null;
const prevVecs = [];

const chkTooClose = (vec, first) => {

  // let prevVecsTmp = (first ? prevVecs : prevVecs.slice(0,-1));
  // checking against all previous vecs (slow way)
  for(const prevVec of prevVecs) {
    if(debug) showVec('- checking prev vec', prevVec);

    // check exact match either direction with prev vec
    if(debug)  showVec('check exact match, old vec', prevVec);
    if(debug)  showVec('                   new vec', vec);
    if(vecEq(prevVec, vec) || vecRevEq(prevVec, vec)) {
      if(debug) showVec('vec exactly matches prevVec', prevVec);
      return {  // skip vec
        headClose:true, tailClose:true, vec1:null, vec2: null};
    }

    // ------ check ends touching  ------
    // check if vec is extending last, vec tail == lastVec.head
    if(debug) showVec('checking vec extension, lastVec', lastVec);
    if(debug) showVec('                            vec', vec);
    const extendingLastVec = lastVec && (ptEq(vec[0], lastVec[1]));
    if(extendingLastVec) {
      if(debug) console.log('vec is extending last, skipping tail check');
    }
    else {
      if(ptTouchesEnd(vec[0],prevVec)) {
        // vec tail is touching prevVec head or tail
        if(debug)  showVec('tail touches prev', prevVec);
        const backUpPnt = backUpPoint(prevVec, vec, false);
        if(!backUpPnt) return {
          headClose:false, tailClose:true, vec1:null, vec2:null};
        return {
          headClose:false, tailClose:true, 
          vec1:[backUpPnt, vec[1]], vec2:null};
      }
    }
    if(ptTouchesEnd(vec[1],prevVec)) {
      // vec head is touching prevVec head or tail
      if(debug)  showVec('head touching prev', prevVec);
      const backUpPnt = backUpPoint(prevVec, vec, true);
      if(!backUpPnt) return {
        headClose:true, tailClose:false, vec1:null, vec2:null};
      return {
        headClose:true, tailClose:false, 
        vec1:[vec[0], backUpPnt], vec2:null};
    }

    // ------ check vecs intersection  ------
    const intPt = (intersectionPoint(vec, prevVec));
    if(intPt) {
      // vec intersects an old vec
      // and neither end point of vec is int point
      // split vec in two with both vecs far enough away
      if(debug) console.log('intersected, at', intPt[0], intPt[1]);
      if(debug)  showVec('      prevVec', prevVec);
      if(debug)  showVec('      vec', vec);

      let vec1 = null;
      const backUpPt1 = backUpPoint(prevVec, [intPt, vec[0]]);
      if(backUpPt1) vec1 = [vec[0], backUpPt1];
      console.log('intersected, backUpPt1', backUpPt1);

      let vec2 = null;
      const backUpPt2 = backUpPoint(prevVec, [intPt, vec[1]]);
      if(backUpPt2) vec2 = [backUpPt2, vec[1]];
      console.log('             backUpPt2', backUpPt2);

      return {headClose:true, tailClose:true, vec1, vec2};
    }

    // ------ check for either vec end point too close ------
    // tail point of vec only checked on ...
    //   first vector and not extendingLastVec
    if(first && !extendingLastVec) {

      if (debug) console.log('starting tail dist chk');
      const dist2prev = distPntToVec(vec[0], prevVec);
      if(dist2prev < (2 * radius)) {
        // tail end point too close to an old vec
        // back up to point on vec far enough away
        let vec1 = null;
        const backUpPt = backUpPoint(prevVec, vec, false);
        console.log('tail too close, backUpPoint result', 
                                backUpPt);
        if(backUpPt) {
          console.log('vec tail too close to prev vec');
          return {
            headClose:false, tailClose:true, 
            vec1:[backUpPt, vec[1]], vec2: null};
        }
        else {
          console.log('both ends of vec too close');
          return {  // skip vec
            headClose:true, tailClose:true, 
            vec1:null, vec2: null};
        }
      }
    }
    if(vecEq(prevVec, lastVec)) {
      if (debug) console.log(
        'prevVec == lastVec, skipping head dist check to last vec');
    }
    else {
      if (debug) console.log('starting head dist chk');
      const dist2prev = distPntToVec(vec[1], prevVec);
      if (debug) console.log('head dist is', dist2prev);
      if(dist2prev < (2 * radius)) {
        // head end point too close to an old vec
        // back up to point on vec far enough away
        let vec1 = null;
        const backUpPt = backUpPoint(prevVec, vec, true);
        showVec('head too close to old vec (head, backUpPoint):', 
                                        [vec[1], backUpPt]);
        if(backUpPt) {
          vec1 = [vec[0], backUpPt];
          if(debug)  showVec('head too close to old vec, vec1', vec1);
          return {headClose:true, tailClose:false, vec1, vec2: null};
        }
        else {
          if (debug) showVec('both ends too close, vec:', vec);
          return {headClose:true, tailClose:true,  vec1: null, vec2: null};
        }
      }
    }
  }
  // vec not too close to any prev vec
  return {headClose:false, tailClose:false, vec1: null, vec2: null};
}

const hullChains = [];
let   spherePts  = [];

const addToHullChains = () => {
  if(spherePts.length) {
    const spheres = spherePts.map((pt) =>
      sphere({radius, segments, center: pt.concat(0)}));
    hullChains.push(hullChain(...spheres));
  }
}

const addHull = (vec) => {
  showVec(' - hull', vec);
  if(genHulls) {
    if(spherePts.length && ptEq(spherePts.at(-1), vec[0]))
      spherePts.push(vec[1]);
    else {
      addToHullChains();
      spherePts = [vec[0], vec[1]];
    }
  }
}

holes = [];

const addHole = (tailPoint, headPoint) => {
  showVec(' - hole', [tailPoint, headPoint]);
  if(genHoles) {
    const w       = headPoint[0] - tailPoint[0];
    const h       = headPoint[1] - tailPoint[1];
    const len     = Math.sqrt(w*w + h*h);
    let scale   = plateDepth/len;
    let holeLen = plateDepth*1.414;
    if(debugScale) {
      scale   = .1;
      holeLen = .1;
    }
    const x       = headPoint[0] + scale*w;
    const y       = headPoint[1] + scale*h;
    holes.push( hull(
      sphere({radius:holeTop, segments, 
              center:headPoint.concat(0)}),
      sphere({radius:holeBot, segments, 
              center: [x,y,-holeLen]})));
  }
}

let lastPoint = null;

// returns next seg idx
const handlePoint = (point, segIdx, segLast) => {
  if(segIdx == 0) {
    // first point of segment
    if (debug) console.log('first point of segment', 
                  point[0].toFixed(1), point[1].toFixed(1));
    if (debug) console.log('only setting lastPoint');
    lastPoint = point;
    lastVec   = null;
    return 1; // next segidx is 1
  }
  // not first point
  let vec = [lastPoint, point];
  console.log('\n-- handlePoint segIdx, segLast:', segIdx, segLast);
  showVec(    '                       vec:', vec);

  // if(lastVec && ptEq(vec[0],lastVec[1]) && ptEq(vec[1],lastVec[0])) {
  //   // vec is reverse of last vec
  //   // these fonts go back over path a second time
  //   // we can ignore all remaining points/vecs in this segment
  //   console.log("\npath is reversing -- ignore rest of segment\n");
  //   if(debug) showVec('add last hole to lastVec', lastVec);
  //     addHole(lastVec[0], lastVec[1]);
  //   return null;
  // }

  const {headClose, tailClose, vec1, vec2} = 
                       chkTooClose(vec, (segIdx == 1));
  if(headClose || tailClose) { 
    // ---- something was too close
    if (debug) console.log('====== something was too close ======');
    if(debug)  showVec('too close result, vec1', vec1);
    if(debug)  showVec('                  vec2', vec2);

    if(vec1 == null) {
      // ---- both ends too close, skipping vec -----
      if(debug) showVec('both ends too close, skipping vec', vec);
      lastPoint = null;
      if(lastVec) {
        if(debug) showVec('add last hole to lastVec', lastVec);
        addHole(lastVec[0], lastVec[1]);
      }
      return 0; // next segidx is 0
    }

    // ---- an end was too close
    if(segIdx == 1 || tailClose) 
      addHole(point, vec1[0]); // add first hole
    addHull(vec1);
    if(debug) showVec('adding to prevVecs vec1', vec1);
    prevVecs.push(vec1);
    if(vec2) {
      // had intersetion
      // vec was split into vec1 and vec2
      // handle vec2 as first in new segment
      // recursive
      lastPoint = vec2[0];
      handlePoint(vec2[1], 1, segLast);
      if(!segLast) return 2;  // next segidx is 2
    }
    if(debug) {
      showVec('checking last hole',vec1);
      console.log({segLast, headClose});
    } 
    if(segLast || headClose) 
      addHole(vec1[0], vec1[1]); // add last hole
    lastPoint = vec[1];
    return segIdx + 1; // next segidx
  }

  // ---- point was not too close
  if(debug) showVec('  -- not too close', vec);
  if(segIdx == 1) addHole(point, lastPoint); // add first hole
  addHull(vec);
  if(segLast) addHole(lastPoint, point); // add last hole
  if(debug)  showVec('adding to prevVecs vec', vec);
  prevVecs.push(vec);
  lastPoint = point;
  lastVec   = vec;
  return segIdx + 1; // next segidx
}

// flat-head M3 bolt
const fhBolt_M3 = (length) => {
  const plasticOfs  = 0.3
  const topRadius   = 6/2  + plasticOfs;
  const shaftRadius = 3/2  + plasticOfs;
  const topH        = 1.86 + plasticOfs;
  const zOfs        = -topH/2;
  const cylH        = length - topH + 1; // 1mm excess
  const cylZofs     = -cylH/2 - topH;
  const top = translateZ(zOfs, cylinderElliptic(
                {startRadius:[shaftRadius, shaftRadius], 
                  endRadius:[topRadius,topRadius],
                  height:topH}));
  const shaft = translateZ(cylZofs, cylinder(
                  {height:cylH, radius:shaftRadius}));
  return union(top, shaft);
};

const getParameterDefinitions = () => {
  return [
    { name: 'fontIdx', type: 'choice', caption: 'Font:', initial: fontIdx, 
      // values: [...localFonts.keys()],
      // captions: localFonts.map(font => font.name)
      values: fontValues, captions: fontCaptions
    },
    { name: 'text', type: 'text', initial: text, 
      size: 10, maxLength: 6, caption: 'Display Text:', 
      placeholder: 'Enter 3 to 6 characters' 
    },
    { name: 'fontsizeAdj', type: 'number', 
      initial: fontsizeAdj, min: 0.25, max: 4.0, 
      step: 0.1, caption: 'Font Size:' 
    },
    { name: 'vertOfs', type: 'int', 
      initial: vertOfs, min: -plateH, max: plateH, 
      step: 1, caption: 'Vertical Offset:' 
    },
    { name: 'show', type: 'choice', caption: 'Show:', 
      values: [0, 1, 2], initial: 1,
      captions: ['Plate With Cutouts', 
                 'Channels and Holes', 
                 'Only Channels']
    },
  ];
}

const main = (params) => {
  const {fontIdx, text, vertOfs, fontsizeAdj} = params;

  const font = localFonts[fontIdx];

  switch(params.show) {
    case 0: genHulls = true; genHoles = true; 
            genPlate = true; break;
    case 1: genHulls = true; genHoles = true; 
            genPlate = false; break;
    case 2: genHulls = true; genHoles = false; 
            genPlate = false; break;
  }
  console.log("---- main ----");

  let strWidth  = 0;
  let strHeight = 0;
  for(const char of text) {
    const {width, height} = vectorChar({font}, char);
    strWidth  += width;
    strHeight = Math.max(strHeight, height);
  };

  if (debug) console.log({strWidth, strHeight});
  
  const scaleW    = (plateW - padSides*2)  / strWidth;
  const scaleH    = (plateH - padTopBot*2) / strHeight;
  let   textScale = Math.min(scaleW, scaleH) * fontsizeAdj;

  strWidth  *= textScale;
  strHeight *= textScale;
  let xOfs   = (plateW - strWidth)/2  - plateW/2;
  let yOfs   = (plateH - strHeight)/2 - plateH/2 + vertOfs;

  if (debugScale) {
    textScale = 1;
    xOfs      = 0;
    yOfs      = 0
  }

  strWidth  = 0;
  for(const char of text) {
    const charRes = vectorChar({font, xOffset:strWidth}, char);
    const {width, segments:segs} = charRes;
    console.log("\n======== CHAR: " + char + 
                 ', segment count:', segs.length);
    strWidth  += width;
    segs.forEach( seg => {
      console.log("\n--- seg ---, point count: ", seg.length);
      let segIdx = 0;
      seg.every( point => {
        point[0] *= textScale;
        point[1] *= textScale;
        segIdx = handlePoint(point, segIdx, segIdx == seg.length-1);
        return (segIdx != null);  // if null, break from both loops
      });
    });
  };
  console.log("\n---- end ----, hole count:", holes.length);

  addToHullChains(); // add remaining spheres to hullchains
  const allHulls = hullChains.concat(holes);
  const zOfs     =  plateDepth/2 - textZofs*radius;
  const hullsOfs = translate([xOfs, yOfs, zOfs], allHulls);

  if(!genPlate) return hullsOfs;
  
  const plate = cuboid({size: [plateW, plateH, plateDepth]});

  const notchedPlate = subtract(plate, 
      translate( [-plateW/2 + filetRadius/2, plateH/2 - filetRadius/2, 0],
        cuboid({size: [filetRadius, filetRadius, plateDepth]})),     
      translate( [ plateW/2 - filetRadius/2, plateH/2 - filetRadius/2, 0],
        cuboid({size: [filetRadius, filetRadius, plateDepth]})));      

  const roundedPlate = union(notchedPlate,    
    cylinder({
      center:[-plateW/2 + filetRadius, plateH/2 - filetRadius, 0],
      height:plateDepth, radius:filetRadius}), 
    cylinder({
      center:[ plateW/2 - filetRadius, plateH/2-filetRadius, 0],
      height:plateDepth, radius:filetRadius}));;

  const boltOfs = 5.25;

  const boltUL = translate(
      [-plateW/2 + boltOfs,  plateH/2 - boltOfs, plateDepth/2],
      fhBolt_M3(plateDepth + 1));
  const boltUM = translate(
      [                  0,  plateH/2 - boltOfs, plateDepth/2],
      fhBolt_M3(plateDepth + 1));
  const boltUR = translate(
      [ plateW/2 - boltOfs,  plateH/2 - boltOfs, plateDepth/2],
      fhBolt_M3(plateDepth + 1));
  const boltBL = translate(
      [-plateW/2 + boltOfs, -plateH/2 + boltOfs, plateDepth/2],
      fhBolt_M3(plateDepth + 1));
  const boltBM = translate(
      [                  0, -plateH/2 + boltOfs, plateDepth/2],
      fhBolt_M3(plateDepth + 1));
  const boltBR = translate(
      [ plateW/2 - boltOfs, -plateH/2 + boltOfs, plateDepth/2],
      fhBolt_M3(plateDepth + 1));

  const plateOut = subtract(roundedPlate, hullsOfs, 
                            boltUL, boltUM, boltUR, boltBL, boltBM, boltBR);

  return plateOut;
};
module.exports = {main, getParameterDefinitions};

const localFonts = [
{name:'HersheySans1',height:662,
32:[378,],

33:[315,  315,662,  315,2,  315,63,  284,3,],

38:[504,220,662,220,4,472,662,472,4,],
35:[662,220,378,662,3,189,189,630,1,],36:[630,630,567,567,6,],37:[756,756,662,189,0,346,662,410,5,630,220,567,1,],38:[819,819,378,819,4,],38:[315,252,598,220,6,],40:[441,441,788,378,7,],41:[441,189,788,252,7,],42:[504,346,472,346,9,189,378,504,1,504,378,189,1,],43:[819,504,567,504,0,220,284,788,2,],44:[252,252,126,220,9,],45:[819,220,284,788,2,],46:[252,220,158,189,1,],47:[693,],48:[630,378,662,284,6,],49:[630,284,536,346,5,],50:[630,220,504,220,5,],51:[630,252,662,598,6,],52:[630,504,662,189,2,504,662,504,0,],53:[630,567,662,252,6,],54:[630,598,567,567,6,],55:[630,630,662,315,0,189,662,630,6,],56:[630,346,662,252,6,],57:[630,598,441,567,3,],58:[252,220,378,189,3,220,158,189,1,],59:[252,220,378,189,3,252,126,220,9,],38:[756,724,567,220,2,],61:[819,220,378,788,3,220,189,788,1,],38:[756,220,567,724,2,],63:[567,189,504,189,5,378,63,346,3,],64:[850,662,410,630,4,472,504,410,4,662,504,630,2,693,504,662,2,],65:[567,378,662,126,0,378,662,630,0,220,220,536,2,],66:[662,220,662,220,0,220,662,504,6,220,346,504,3,],67:[662,662,504,630,5,],68:[662,220,662,220,0,220,662,441,6,],69:[598,220,662,220,0,220,662,630,6,220,346,472,3,220,0,630,0,],70:[567,220,662,220,0,220,662,630,6,220,346,472,3,],71:[662,662,504,630,5,504,252,662,2,],72:[693,220,662,220,0,662,662,662,0,220,346,662,3,],73:[252,220,662,220,0,],74:[504,472,662,472,1,],75:[662,220,662,220,0,662,662,220,2,378,378,662,0,],76:[536,220,662,220,0,220,0,598,0,],77:[756,220,662,220,0,220,662,472,0,724,662,472,0,724,662,724,0,],78:[693,220,662,220,0,220,662,662,0,662,662,662,0,],79:[693,378,662,315,6,],80:[662,220,662,220,0,220,662,504,6,],81:[693,378,662,315,6,],82:[662,220,662,220,0,220,662,504,6,441,346,662,0,],83:[630,630,567,567,6,],84:[504,346,662,346,0,126,662,567,6,],85:[693,220,662,220,1,],86:[567,126,662,378,0,630,662,378,0,],87:[756,158,662,315,0,472,662,315,0,472,662,630,0,788,662,630,0,],88:[630,189,662,630,0,630,662,189,0,],89:[567,126,662,378,3,630,662,378,3,],90:[630,630,662,189,0,189,662,630,6,189,0,630,0,],91:[441,220,788,441,7,],92:[441,],93:[441,189,788,410,7,],94:[504,346,724,598,2,],95:[567,],96:[252,252,504,189,4,],97:[598,567,441,567,0,567,346,504,4,],98:[598,220,662,220,0,220,346,284,4,],99:[567,567,346,504,4,],100:[598,567,662,567,0,567,346,504,4,],101:[567,189,252,567,2,],102:[378,410,662,346,6,158,441,378,4,],103:[598,567,346,504,4,],104:[598,220,662,220,0,220,315,315,4,],105:[252,189,662,220,6,220,441,220,0,],106:[315,252,662,284,6,],107:[536,220,662,220,0,536,441,220,1,346,252,567,0,],108:[252,220,662,220,0,],109:[945,220,441,220,0,220,315,315,4,567,315,662,4,],110:[598,220,441,220,0,220,315,315,4,],111:[598,346,441,284,4,],112:[598,220,346,284,4,],113:[598,567,346,504,4,],114:[410,220,441,220,0,220,252,252,3,],115:[536,536,346,504,4,],116:[378,252,662,252,1,158,441,378,4,],117:[598,220,441,220,1,567,441,567,0,],118:[504,158,441,346,0,536,441,346,0,],119:[693,189,441,315,0,441,441,315,0,441,441,567,0,693,441,567,0,],120:[536,189,441,536,0,536,441,189,0,],121:[504,158,441,346,0,536,441,346,0,],122:[536,536,441,189,0,189,441,536,4,189,0,536,0,],123:[441,378,788,315,7,315,756,284,6,284,252,346,1,],124:[252,],125:[441,252,788,315,7,315,756,346,6,346,252,284,1,],126:[756,189,189,189,2,189,252,220,3,],38:[378,],38:[315,252,0,252,4,252,598,284,6,],38:[176,567,346,504,4,],38:[567,126,662,378,3,630,662,378,3,236,158,520,1,236,252,520,2,],38:[null,236,709,236,4,],38:[567,279,693,247,6,540,693,509,6,],38:[693,378,662,315,6,551,410,536,4,],38:[299,410,850,410,6,410,803,378,8,],38:[454,416,454,214,2,],38:[693,378,662,315,6,331,488,331,1,331,488,472,4,441,331,551,1,],38:[249,236,765,217,7,],38:[655,857,643,857,1,],38:[315,236,882,236,8,],38:[315,252,961,425,9,],38:[378,431,758,305,5,],38:[363,340,416,302,3,],38:[315,268,898,299,9,],38:[249,236,765,217,7,],38:[454,214,454,416,2,],38:[680,170,699,208,7,775,397,586,1,775,397,775,0,],38:[680,170,699,208,7,605,302,605,3,],38:[680,151,775,359,7,775,397,586,1,775,397,775,0,],38:[567,567,158,567,1,378,598,410,6,],38:[567,378,662,126,0,378,662,630,0,220,220,536,2,],38:[567,378,662,126,0,378,662,630,0,220,220,536,2,506,977,380,8,],38:[567,378,662,126,0,378,662,630,0,220,220,536,2,274,810,381,9,],38:[567,378,662,126,0,378,662,630,0,220,220,536,2,227,775,227,8,227,813,246,8,],38:[567,378,662,126,0,378,662,630,0,220,220,536,2,281,914,249,8,538,914,507,8,],38:[567,378,662,126,0,378,662,630,0,220,220,536,2,],38:[598,220,662,220,0,220,662,630,6,220,346,472,3,220,0,630,0,],38:[598,220,662,220,0,220,662,630,6,220,346,472,3,220,0,630,0,519,993,393,8,],38:[598,220,662,220,0,220,662,630,6,220,346,472,3,220,0,630,0,290,810,397,9,],38:[598,220,662,220,0,220,662,630,6,220,346,472,3,220,0,630,0,274,914,242,8,514,914,482,8,],38:[252,220,662,220,0,],38:[252,220,662,220,0,],38:[252,220,662,220,0,117,810,224,9,],38:[252,220,662,220,0,373,914,342,8,],38:[662,110,252,394,2,220,662,220,0,220,662,441,6,],38:[693,220,662,220,0,220,662,662,0,662,662,662,0,252,775,252,8,252,813,271,8,],38:[693,378,662,315,6,],38:[693,378,662,315,6,],38:[693,378,662,315,6,337,810,444,9,],38:[693,378,662,315,6,252,775,252,8,252,813,271,8,],38:[693,378,662,315,6,317,914,285,8,565,914,534,8,],38:[536,189,441,536,0,536,441,189,0,],38:[693,378,662,315,6,],38:[693,220,662,220,1,],38:[693,220,662,220,1,],38:[693,220,662,220,1,337,810,444,9,],38:[693,220,662,220,1,309,914,278,8,573,914,541,8,],38:[567,126,662,378,3,630,662,378,3,],38:[598,567,441,567,0,567,346,504,4,331,740,422,6,],38:[598,567,441,567,0,567,346,504,4,531,809,405,6,],38:[598,567,441,567,0,567,346,504,4,290,621,397,7,],38:[598,567,441,567,0,567,346,504,4,233,586,233,6,233,624,252,6,],38:[598,567,441,567,0,567,346,504,4,299,693,268,6,551,693,520,6,],38:[598,567,441,567,0,567,346,504,4,397,860,378,8,],38:[567,189,252,567,2,315,740,406,6,],38:[567,189,252,567,2,517,806,391,6,],38:[567,189,252,567,2,274,621,381,7,],38:[567,189,252,567,2,291,693,259,6,528,693,497,6,],38:[252,220,441,220,0,158,740,249,6,],38:[252,220,441,220,0,368,762,242,5,],38:[252,220,441,220,0,117,621,224,7,],38:[252,220,441,220,0,378,693,347,6,],38:[598,220,441,220,0,220,315,315,4,233,586,233,6,233,624,252,6,],38:[598,346,441,284,4,331,740,422,6,],38:[598,346,441,284,4,519,808,393,6,],38:[598,346,441,284,4,290,621,397,7,],38:[598,346,441,284,4,233,586,233,6,233,624,252,6,],38:[598,346,441,284,4,295,693,263,6,556,693,524,6,],38:[614,378,567,346,5,378,252,346,2,165,378,591,3,],38:[598,346,441,284,4,],38:[598,220,441,220,1,567,441,567,0,331,740,422,6,],38:[598,220,441,220,1,567,441,567,0,535,784,409,6,],38:[598,220,441,220,1,567,441,567,0,290,621,397,7,],38:[598,220,441,220,1,567,441,567,0,295,693,263,6,556,693,524,6,],38:[504,158,441,346,0,536,441,346,0,480,779,354,6,],38:[504,158,441,346,0,536,441,346,0,256,693,225,6,500,693,468,6,],38:[614,165,284,591,2,],38:[819,220,284,788,2,],38:[402,213,652,184,5,354,652,326,5,],38:[402,184,567,170,5,326,567,312,5,],38:[302,],38:[302,],38:[662,662,504,630,5,142,236,425,2,142,331,425,3,],38:[567,126,662,378,3,630,662,378,3,290,914,258,8,529,914,498,8,],38:[567,378,662,126,0,378,662,630,0,220,220,536,2,],38:[598,220,662,220,0,220,662,630,6,220,346,472,3,220,0,630,0,],38:[662,662,504,630,5,504,252,662,2,],38:[252,220,662,220,0,],38:[693,378,662,315,6,],38:[693,220,662,220,1,],38:[598,567,441,567,0,567,346,504,4,],38:[567,189,252,567,2,],38:[598,567,346,504,4,],38:[252,220,441,220,0,],38:[598,346,441,284,4,],38:[598,220,441,220,1,567,441,567,0,],38:[504,],38:[662,],38:[252,],38:[662,],38:[252,],38:[630,],38:[910,],38:[567,],38:[536,],38:[662,],38:[910,],38:[536,],}
]

const fontValues   = [...localFonts.keys()];
const fontCaptions = localFonts.map(font => font.name);

console.log({fontValues});

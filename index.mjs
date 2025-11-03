'use strict'

import { createRequire } from "module"
const tkrzw =  createRequire(import.meta.url)('bindings')('tkrzw-node')

export default { polyDBM: tkrzw.polyDBM, polyIndex: tkrzw.polyIndex };

/*import fs from "node:fs"

let tkrzw_config = fs.readFileSync('./tkrzw_config.json', 'utf8');
console.log("tkrzw.polyDBM:", tkrzw.polyDBM, "tkrzw.polyDBM.NOOP:", tkrzw.polyDBM.NOOP, "Object.getOwnPropertyNames:", Object.getOwnPropertyNames(tkrzw.polyDBM))
const db1 = new tkrzw.polyDBM(JSON.parse(tkrzw_config), "db/YaHeidar.tkh");

await db1.append("Heidar(a)", "Mola Amir-al-momenin");
console.log( "1:", await db1.getSimple("Heidar(a)", "Key not_set") );
await db1.process("Heidar(a)", (keyExists, key, value)=>{
    console.log("keyExists:", keyExists, "key:", key, "value:", value);
    //return "THANK YOU MY LORD; PLEASE HELP"//tkrzw.polyDBM.NOOP;
    return tkrzw.polyDBM.NOOP;
    // return tkrzw.polyDBM.REMOVE;
},true);
await db1.append("Heidar(a)", "Mola Amir-al-momenin", ",");
console.log( "2:", await db1.getSimple("Heidar(a)", "Key not_set") );
await db1.append("Heidar(a)", "Mola Amir-al-momenin");
console.log( "3:", await db1.getSimple("Heidar(a)", "Key not_set") );
*/


/*import fs from "node:fs"

let tkrzw_config = fs.readFileSync("./tkrzw_index_config.json", 'utf8');
const idx1 = new tkrzw.polyIndex(tkrzw_config, "./db/index_YaHeidar.tkt");
await idx1.add("Imam 1", "Heidar(a)");
await idx1.add("Imam 2", "Hassan(a)");
await idx1.add("Imam 3", "Hussain(a)");

await idx1.makeJumpIterator("Imam");

let res;
while( true )
{
    try
    {
        console.log( await idx1.getIteratorValue() );
        await idx1.continueIteration();
    }
    catch(err){
        console.error(err); break;
    }
}

idx1.freeIterator();
idx1.close();*/

/*var fs = require('fs');
let tkrzw_config = fs.readFileSync('./tkrzw_config.json', 'utf8');
const db1 = new tkrzw.polyDBM(JSON.parse(tkrzw_config), "YaHeidar.tkh");*/


/*const db1 = new tkrzw.polyDBM(require("./tkrzw_config.json"), "./db/YaHeidar.tkh");
const idx1 = new tkrzw.polyIndex(require("./tkrzw_index_config.json"), "./db/index_YaHeidar.tkt");

Promise.allSettled([
db1.set("Heidar(a)", "Mola Amir-al-momenin").then(()=>{idx1.add("Mola", "Heidar(a)"); console.log("SET1 DONE!")}),
db1.set("Heidar(b)", "Mola Abu-Torab").then(()=>{idx1.add("Mola", "Heidar(b)"); console.log("SET2 DONE!")}),
db1.getSimple("Heidar(b)", "Key not_set").then((val)=>console.log("GET3: ",val)),
db1.getSimple("Heidar(a)", "Key not_set").then((val)=>console.log("GET1: ",val)),
db1.getSimple("non existant key", "Key not_set").then((val)=>console.log("GET2: ",val)).catch((val)=>console.log("GET2: ",val)),
db1.shouldBeRebuilt().then(()=>db1.rebuild(require("./tkrzw_rebuild_config.json"))).catch((val)=>console.log("shouldBeRebuilt: ",val)),
idx1.shouldBeRebuilt().then(()=>idx1.rebuild()).catch((val)=>console.log("index shouldBeRebuilt: ",val))
]).then(
    //()=>db1.close()
    ()=>{
    console.log("Good things happened!");
    idx1.check("Mola", "Heidar(a)").then(()=>{console.log("Removing: ", "Heidar(a)"); idx1.remove("Mola", "Heidar(a)").then(()=>{console.log("Removed!! (a)");
        idx1.getValues("Mola", 0).then(res=>{console.log("getValues: ",res); 
            db1.getSimple(res[0], "Key not_set").then((val)=>console.log("getValues[0]: ",val))
        });
    })}).catch(err=>console.log("Check: ", err));
    idx1.check("Mola", "Heidar(c)").then(()=>{console.log("Removing: ", "Heidar(c)"); idx1.remove("Mola", "Heidar(c)").then(()=>{console.log("Removed!! (c)")})}).catch(err=>console.log("Check: ", err));
});
*/

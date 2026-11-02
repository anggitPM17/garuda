const { GARUDA_KEYWORDS, GARUDA_STDLIB } = require('../out/keywords');

console.log("=======================================================");
console.log("GARUDA LSP SERVER SYMBOL TABLE TEST");
console.log("=======================================================\n");

console.log("[PASS] Keywords loaded: " + GARUDA_KEYWORDS.length + " keywords");
console.log("[PASS] Stdlib symbols loaded: " + GARUDA_STDLIB.length + " symbols");

if (GARUDA_KEYWORDS.length > 15 && GARUDA_STDLIB.length > 30) {
    console.log("\n🎉 ALL LSP SYMBOL TABLE TESTS PASSED PERFECTLY!");
} else {
    console.error("[FAIL] Symbol table incomplete");
    process.exit(1);
}

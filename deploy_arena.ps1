# =====================================================================
# SKRIP DEPLOYMENT OTOMATIS POWERSHELL — GARUDA ARENA
# =====================================================================

$ErrorActionPreference = "Stop"

$VPS_IP = "36.50.77.230"
$VPS_USER = "root"
$LOCAL_ROOT = "D:\richat\garuda_v67_0_production\garuda_v67.0.0"

Write-Host "[0/8] Membuat folder tujuan di VPS..." -ForegroundColor Cyan
ssh -o StrictHostKeyChecking=no "${VPS_USER}@${VPS_IP}" "mkdir -p /opt/garuda/garuda_v67_0_production/garuda_v67.0.0/include/garuda/game /opt/garuda/garuda_v67_0_production/garuda_v67.0.0/include/garuda/nusantara /opt/garuda/garuda_v67_0_production/garuda_v67.0.0/include/garuda/richat /opt/garuda/garuda_v67_0_production/garuda_v67.0.0/src /var/www/richat/site/assets"

Write-Host "[1/8] Mengirim nusantara_protocol.hpp ke VPS..." -ForegroundColor Cyan
scp -o StrictHostKeyChecking=no "$LOCAL_ROOT\include\garuda\nusantara\nusantara_protocol.hpp" "${VPS_USER}@${VPS_IP}:/opt/garuda/garuda_v67_0_production/garuda_v67.0.0/include/garuda/nusantara/"

Write-Host "[2/8] Mengirim nusantara_broker.hpp ke VPS..." -ForegroundColor Cyan
scp -o StrictHostKeyChecking=no "$LOCAL_ROOT\include\garuda\nusantara\nusantara_broker.hpp" "${VPS_USER}@${VPS_IP}:/opt/garuda/garuda_v67_0_production/garuda_v67.0.0/include/garuda/nusantara/"

Write-Host "[3/8] Mengirim arena.hpp ke VPS..." -ForegroundColor Cyan
scp -o StrictHostKeyChecking=no "$LOCAL_ROOT\include\garuda\game\arena.hpp" "${VPS_USER}@${VPS_IP}:/opt/garuda/garuda_v67_0_production/garuda_v67.0.0/include/garuda/game/"

Write-Host "[4/8] Mengirim richat_gateway.hpp ke VPS..." -ForegroundColor Cyan
scp -o StrictHostKeyChecking=no "$LOCAL_ROOT\include\garuda\richat\richat_gateway.hpp" "${VPS_USER}@${VPS_IP}:/opt/garuda/garuda_v67_0_production/garuda_v67.0.0/include/garuda/richat/"

Write-Host "[5/8] Mengirim richat_server.cpp ke VPS..." -ForegroundColor Cyan
scp -o StrictHostKeyChecking=no "$LOCAL_ROOT\src\richat_server.cpp" "${VPS_USER}@${VPS_IP}:/opt/garuda/garuda_v67_0_production/garuda_v67.0.0/src/"

Write-Host "[6/8] Mengirim game.html ke VPS..." -ForegroundColor Cyan
scp -o StrictHostKeyChecking=no "$LOCAL_ROOT\site\assets\game.html" "${VPS_USER}@${VPS_IP}:/var/www/richat/site/assets/"

Write-Host "[7/8] Mengirim index.html ke VPS..." -ForegroundColor Cyan
scp -o StrictHostKeyChecking=no "$LOCAL_ROOT\site\index.html" "${VPS_USER}@${VPS_IP}:/var/www/richat/site/"

Write-Host "[8/8] Mengompilasi ulang C++ server & restart service di VPS..." -ForegroundColor Yellow
Write-Host "(Ini mungkin memakan waktu beberapa detik, harap tunggu...)" -ForegroundColor Yellow

$ssh_command = "cd /opt/garuda/garuda_v67_0_production/garuda_v67.0.0/build && make garuda_richat_server -j2 && systemctl restart richat-garuda && echo '✅ Kompilasi dan Deploy Berhasil!'"
ssh -o StrictHostKeyChecking=no "${VPS_USER}@${VPS_IP}" $ssh_command

Write-Host "[SELESAI] Silakan buka https://richat.my.id/assets/game.html" -ForegroundColor Green

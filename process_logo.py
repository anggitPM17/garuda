# =====================================================================
# GARUDA COMPILER — LOGO & FAVICON ASSET PROCESSOR
# =====================================================================

import os
import io
import base64
from PIL import Image, ImageDraw, ImageFont, ImageFilter
import numpy as np

def process_logo():
    src_path = 'garuda icon.jfif'
    if not os.path.exists(src_path):
        print(f"Error: {src_path} not found!")
        return

    print(f"Loading {src_path}...")
    img = Image.open(src_path).convert('RGBA')
    arr = np.array(img, dtype=np.float32)

    # Calculate distance from white [255, 255, 255]
    dist = np.sqrt((255.0 - arr[:,:,0])**2 + (255.0 - arr[:,:,1])**2 + (255.0 - arr[:,:,2])**2)

    # Smooth alpha transition
    thresh_low = 18.0
    thresh_high = 65.0
    alpha = np.clip((dist - thresh_low) / (thresh_high - thresh_low), 0.0, 1.0) * 255.0
    arr[:,:,3] = alpha

    # Create RGBA Image
    transparent_img = Image.fromarray(arr.astype(np.uint8), 'RGBA')

    # Get tight bounding box of logo content
    bbox = transparent_img.getbbox()
    if bbox:
        cropped = transparent_img.crop(bbox)
    else:
        cropped = transparent_img

    # Place in square canvas with 3% margin
    max_side = max(cropped.width, cropped.height)
    canvas_size = int(max_side * 1.06)
    square_canvas = Image.new('RGBA', (canvas_size, canvas_size), (0, 0, 0, 0))
    offset = ((canvas_size - cropped.width) // 2, (canvas_size - cropped.height) // 2)
    square_canvas.paste(cropped, offset, cropped)

    # Master 2048x2048
    master_logo = square_canvas.resize((2048, 2048), Image.Resampling.LANCZOS)

    # Ensure output directories
    os.makedirs('site/assets', exist_ok=True)

    # 1. Master PNG
    master_logo.save('site/assets/logo-master.png', 'PNG')
    print("✓ Saved site/assets/logo-master.png (2048x2048)")

    # 2. Main Logo 512x512
    logo_512 = master_logo.resize((512, 512), Image.Resampling.LANCZOS)
    logo_512.save('site/assets/logo.png', 'PNG')
    print("✓ Saved site/assets/logo.png (512x512)")

    # 3. Hero Logo 256x256
    logo_256 = master_logo.resize((256, 256), Image.Resampling.LANCZOS)
    logo_256.save('site/assets/logo-hero.png', 'PNG')
    print("✓ Saved site/assets/logo-hero.png (256x256)")

    # 4. Favicon 128x128 & 64x64
    logo_128 = master_logo.resize((128, 128), Image.Resampling.LANCZOS)
    logo_128.save('site/assets/favicon.png', 'PNG')
    print("✓ Saved site/assets/favicon.png (128x128)")

    # 5. Favicon ICO (multi-size: 16, 32, 48, 64)
    ico_sizes = [(16, 16), (32, 32), (48, 48), (64, 64)]
    logo_512.save('site/assets/favicon.ico', format='ICO', sizes=ico_sizes)
    print("✓ Saved site/assets/favicon.ico (Multi-size ICO)")

    # 6. Favicon SVG wrapper with Data URI
    buffered = io.BytesIO()
    logo_256.save(buffered, format="PNG")
    img_b64 = base64.b64encode(buffered.getvalue()).decode("utf-8")
    svg_content = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256" width="100%" height="100%">
  <image href="data:image/png;base64,{img_b64}" x="0" y="0" width="256" height="256"/>
</svg>'''
    with open('site/assets/favicon.svg', 'w', encoding='utf-8') as f:
        f.write(svg_content)
    print("✓ Saved site/assets/favicon.svg (Embedded Data-URI SVG)")

    # 7. OpenGraph Social Image (1200x630)
    og_img = Image.new('RGBA', (1200, 630), (15, 23, 42, 255)) # Dark slate background #0f172a
    draw = ImageDraw.Draw(og_img)

    # Red accent border line at top
    draw.rectangle([(0, 0), (1200, 8)], fill=(220, 38, 38, 255)) # Red #dc2626

    # Paste Logo on left side
    logo_og = master_logo.resize((380, 380), Image.Resampling.LANCZOS)
    og_img.paste(logo_og, (100, 125), logo_og)

    # Draw Title & Text
    try:
        font_title = ImageFont.truetype("arial.ttf", 64)
        font_sub = ImageFont.truetype("arial.ttf", 32)
        font_tag = ImageFont.truetype("arial.ttf", 24)
    except:
        font_title = ImageFont.load_default()
        font_sub = ImageFont.load_default()
        font_tag = ImageFont.load_default()

    draw.text((520, 180), "GARUDA BEAM", fill=(248, 250, 252, 255), font=font_title)
    draw.text((520, 265), "Compiler & Runtime System", fill=(220, 38, 38, 255), font=font_sub)
    draw.text((520, 325), "High-Performance Actor-Model VM for Indonesia 🇮🇩", fill=(148, 163, 184, 255), font=font_tag)

    # Save OpenGraph image
    og_img.convert('RGB').save('site/assets/og-image.png', 'PNG')
    print("✓ Saved site/assets/og-image.png (1200x630 OpenGraph Preview)")

    # 8. Copy to other project directories if they exist
    extra_dirs = [
        'landing-page/assets',
        'docs/assets',
        'stadion-garuda/frontend/public'
    ]
    for d in extra_dirs:
        if os.path.exists(os.path.dirname(d)):
            os.makedirs(d, exist_ok=True)
            logo_512.save(os.path.join(d, 'logo.png'), 'PNG')
            logo_512.save(os.path.join(d, 'favicon.ico'), format='ICO', sizes=ico_sizes)
            with open(os.path.join(d, 'favicon.svg'), 'w', encoding='utf-8') as f:
                f.write(svg_content)
            print(f"✓ Copied assets to {d}/")

    print("\n🎉 All Garuda logo assets generated and deployed successfully!")

if __name__ == '__main__':
    process_logo()

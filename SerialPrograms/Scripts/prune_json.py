import json

filling_slugs = [
            "apple",
            "avocado",
            "bacon",
            "baguette",
            "banana",
            "basil",
            "cheese",
            "cherry-tomatoes",
            "chorizo",
            "cucumber",
            "egg",
            "fried-fillet",
            "green-bell-pepper",
            "ham",
            "hamburger",
            "herbed-sausage",
            "jalapeño",
            "kiwi",
            "klawf-stick",
            "lettuce",
            "noodles",
            "onion",
            "pickle",
            "pineapple",
            "potato-salad",
            "potato-tortilla",
            "prosciutto",
            "red-bell-pepper",
            "red-onion",
            "rice",
            "smoked-fillet",
            "strawberry",
            "tofu",
            "tomato",
            "watercress",
            "yellow-bell-pepper"]

condiment_slugs = [
            "butter",
            "chili-sauce",
            "cream-cheese",
            "curry-powder",
            "horseradish",
            "jam",
            "ketchup",
            "marmalade",
            "mayonnaise",
            "mustard",
            "olive-oil",
            "peanut-butter",
            "pepper",
            "salt",
            "vinegar",
            "wasabi",
            "whipped-cream",
            "yogurt",
            "bitter-herba-mystica",
            "spicy-herba-mystica",
            "salty-herba-mystica",
            "sour-herba-mystica",
            "sweet-herba-mystica"]

pick_slugs = [
            "blue-flag-pick",
            "blue-poke-ball-pick",
            "blue-sky-flower-pick",
            "gold-pick",
            "green-poke-ball-pick",
            "heroic-sword-pick",
            "magical-heart-pick",
            "magical-star-pick",
            "parasol-pick",
            "party-sparkler-pick",
            "pika-pika-pick",
            "red-flag-pick",
            "red-poke-ball-pick",
            "silver-pick",
            "smiling-vee-pick",
            "sunrise-flower-pick",
            "sunset-flower-pick",
            "vee-vee-pick",
            "winking-pika-pick"]

def filter_keys(obj, keys):
    if isinstance(obj, dict):
        return {k: filter_keys(v, keys) for k, v in obj.items() if k not in keys}
    elif isinstance(obj, list):
        return [filter_keys(elem, keys) for elem in obj]
    else:
        return obj

with open("OCR.json", "r", encoding="utf-8") as f:
    input_ocr = json.load(f)

output_ocr_condiments = filter_keys(input_ocr, filling_slugs + pick_slugs)
with open('SandwichCondimentOCR.json', 'w', encoding="utf-8") as f:
    json.dump(output_ocr_condiments, f, indent=4, ensure_ascii=False)

output_ocr_fillings = filter_keys(input_ocr, condiment_slugs + pick_slugs)
with open('SandwichFillingOCR.json', 'w', encoding="utf-8") as f:
    json.dump(output_ocr_fillings, f, indent=4, ensure_ascii=False)

output_ocr_picks = filter_keys(input_ocr, condiment_slugs + filling_slugs)
with open('SandwichPickOCR.json', 'w', encoding="utf-8") as f:
    json.dump(output_ocr_picks, f, indent=4, ensure_ascii=False)



with open("Sprites.json", "r", encoding="utf-8") as f:
    input_sprite = json.load(f)

output_sprite_condiments = filter_keys(input_sprite, filling_slugs + pick_slugs)
with open('SandwichCondimentSprites.json', 'w', encoding="utf-8") as f:
    json.dump(output_sprite_condiments, f, indent=4, ensure_ascii=False)

output_sprite_fillings = filter_keys(input_sprite, condiment_slugs + pick_slugs)
with open('SandwichFillingSprites.json', 'w', encoding="utf-8") as f:
    json.dump(output_sprite_fillings, f, indent=4, ensure_ascii=False)

output_sprite_picks = filter_keys(input_sprite, condiment_slugs + filling_slugs)
with open('SandwichPickSprites.json', 'w', encoding="utf-8") as f:
    json.dump(output_sprite_picks, f, indent=4, ensure_ascii=False)
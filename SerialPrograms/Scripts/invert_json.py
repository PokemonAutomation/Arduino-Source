import json
import sys


def main():
    if len(sys.argv) != 3:
        print("usage: {} <input.json> <output.json>".format(sys.argv[0]))
        sys.exit(1)

    with open(sys.argv[1], "r", encoding="utf-8") as f:
        data = json.load(f)

    inverted = {}
    for key, value in data.items():
        for lang, word in value.items():
            if lang not in inverted:
                inverted[lang] = {}
            inverted[lang][key] = word

    with open(sys.argv[2], "w", encoding="utf-8") as f:
        json.dump(inverted, f, indent=4, ensure_ascii=False)


if __name__ == "__main__":
    main()
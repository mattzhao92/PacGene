import matplotlib.pyplot as plt

def extract_gene(file_name):
    with open(file_name) as f:
        for line in f:
            if '#' in line:
                parts = line.split()
                gene = parts[-1][1:-1]
                print(gene)

extract_gene('trace')




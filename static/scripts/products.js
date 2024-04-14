const products = {
    "1": {
        "name": "GO Engineer Cap",
        "price": 19.95,
        "description": `
            <p>
                This trustworthy engineer cap can make you feel like a true-to-life GO
                Train engineer. Unisex and perfectly fitting and adjustable for all sizes,
                this is a true gift for train enthusiasts.
            </p>
        `,
        "slug": "GO%20Engineer%20Cap"
    },
    "2": {
        "name": "Metrolinx Tumbler",
        "price": 35.56,
        "description": `
            <p>
                Nothing beats a trusty canteen from an organization that keeps all of
                Southern Ontario running. Whether it be on the UP Express, a train,
                or just a bus, you can enjoy your insulated 17oz tumbler.
            </p>
        `,
        "slug": "Metrolinx%20Tumbler"
    },
    "3": {
        "name": "PRESTO Laptop Case",
        "price": 30.50,
        "description": `
            <p>
                Keep your laptop safe and secure in this stylish PRESTO-themed
                laptop case. That way, when you ride the GO, you're ready
                to go with your laptop.
            </p>
        `,
        "slug": "PRESTO%20Laptop%20Case"
    },
    "4": {
        "name": "PRESTO T-shirt",
        "price": 20.00,
        "description": `
            <p>
                Rep your love for PRESTO with our latest brand insigna, a perfect
                complement to your PRESTO card and a comfortable fit for your trip.
            </p>
        `,
        "slug": "PRESTO%20T-shirt"
    }
};

function getProductLink(inputProductId, inputProductObject) {
    return "/products/" + inputProductId + "/" + inputProductObject.slug;
}

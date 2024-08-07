// This JS file acts as a "Template" for the HTML files (since we
// were instructed not to use Crow's templating engine)

// Although I'm decent at Javascript, there's no way I could
// make a function as clean and efficient at getting cookies as
// the one below:
// https://stackoverflow.com/questions/5639346/what-is-the-shortest-function-for-reading-a-cookie-by-name-in-javascript#answer-25490531
const getCookieValue = (name) => (
    document.cookie.match('(^|;)\\s*' + name + '\\s*=\\s*([^;]+)')?.pop() || ''
)

function getHtmlElement() {
    let possibleHtmlElements = document.getElementsByTagName('html');

    if(!possibleHtmlElements || possibleHtmlElements.length < 1) {
        return null;
    }

    return possibleHtmlElements[0];
}

function getBodyElement() {
    // We perform this check, "just in case" the body doesn't exist
    let possibleBodyElements = document.getElementsByTagName('body');

    // If it doesn't, we can't make a navbar
    if(!possibleBodyElements || possibleBodyElements.length < 1) {
        return null;
    }

    return possibleBodyElements[0];
}

function constructHeadTags(elementType, tags) {
    // We'll iterate through each tag and apply all attributes that it contains
    // to a corresponding new element added to the head
    tags.forEach(tag => {
        let currentTag = document.createElement(elementType);

        Object.keys(tag).forEach(key => {
            currentTag.setAttribute(key, tag[key]);
        });

        document.head.appendChild(currentTag);
    });
}

function configureSiteHead(siteHeadElement) {
    let metaTags = [
        {
            "charset": "UTF-8"
        },
        {
            "name": "viewport",
            "content": "width=device-width, initial-scale=1"
        }
    ];

    constructHeadTags("meta", metaTags);

    let linkTags = [
        {
            "href": "/styles/site.css",
            "rel": "stylesheet"
        }
    ];

    constructHeadTags("link", linkTags);
}

function configureSiteNavbarLinks(navbarLinkContainer) {
    let menuLinks = {
        "Home": "/"
    };

    for (const currentProductId in products) {
        let currentProduct = products[currentProductId];
        menuLinks[currentProduct.name] = getProductLink(currentProductId, currentProduct);
    }

    let possibleCartFilledCookie = getCookieValue("cart_filled");

    if(possibleCartFilledCookie && possibleCartFilledCookie === "true") {
        menuLinks["Cart"] = "/cart";
    }

    for(let link in menuLinks) {
        let menuLinkLi = document.createElement('li');
        menuLinkLi.classList.add('main-nav__right__menu__item');
        navbarLinkContainer.appendChild(menuLinkLi);

        let menuLinkElement = document.createElement('a');
        menuLinkElement.setAttribute('href', menuLinks[link]);
        menuLinkElement.innerHTML = link;
        menuLinkLi.appendChild(menuLinkElement);
    }
}

function configureSiteNavbar() {
    let bodyElement = getBodyElement();
    
    // If the body element doesn't exist, don't proceed
    if(!bodyElement) {
        console.error("Navbar could not be injected as the body element could not be found");
        return;
    }

    // Now we'll construct the navbar
    let navbar = document.createElement('nav');
    navbar.setAttribute('id', 'main-nav');
    navbar.classList.add('main-nav');

    // Inside the navbar, we'll have 2 children, the left side for the logo
    // and the right side for the shopping cart
    let navbarLeft = document.createElement('div');
    navbarLeft.classList.add('main-nav__left');
    navbar.appendChild(navbarLeft);

    let navbarLogo = document.createElement('a');
    navbarLogo.setAttribute('href', '/');
    navbarLogo.innerHTML = "The Metro Shop";
    navbarLogo.classList.add('main-nav__left__logo');
    navbarLeft.appendChild(navbarLogo);

    // Now we'll do the right side of the navbar
    let navbarRight = document.createElement('div');
    navbarRight.classList.add('main-nav__right');
    navbar.appendChild(navbarRight);

    let navbarMenu = document.createElement('ul');
    navbarMenu.classList.add('main-nav__right__menu');
    navbarRight.appendChild(navbarMenu);

    configureSiteNavbarLinks(navbarMenu);

    // Next we'll see if we need to inject before the first element, or
    // if the body is empty, we can simply append
    if(bodyElement.firstChild) {
        bodyElement.insertBefore(navbar, bodyElement.firstChild);
    } else {
        bodyElement.appendChild(navbar);
    }
}

function configureSiteFooter() {
    let bodyElement = getBodyElement();
    
    // If the body element doesn't exist, don't proceed
    if(!bodyElement) {
        console.error("Footer couldn't be injected as it could not find the body element");
        return;
    }
    
    let footer = document.createElement('footer');
    footer.setAttribute('id', 'main-footer');
    footer.classList.add('main-footer');
    bodyElement.appendChild(footer);
    
    // In the footer we'll simply put a disclaimer and who
    // made the site
    let footerLeft = document.createElement('div');
    footerLeft.classList.add('main-footer__left');
    footer.appendChild(footerLeft);
    
    let footerLeftParagraph = document.createElement('p');
    footerLeftParagraph.innerHTML = "This site uses Metrolinx's products and images. This is a project for a class at Conestoga College, and this site and its contents are not endorsed nor affiliated with Metrolinx, a Crown organization of Ontario.";
    footerLeft.appendChild(footerLeftParagraph);

    let footerRight = document.createElement('div');
    footerRight.classList.add('main-footer__right');
    footer.appendChild(footerRight);

    let footerRightParagraph = document.createElement('p');
    footerRightParagraph.innerHTML = "This site was built by Luna Parker for CSCN72050, Lab 10, in Winter 2024.";
    footerRight.appendChild(footerRightParagraph);
}

// We'll wait for the document to be fully loaded before populating data
// in the HTML DOM
document.addEventListener("DOMContentLoaded", (event) => {
    // If we don't have products injected, the rest of the generation code
    // won't work, so we'll just return an error
    if(!products) {
        console.error("Failed to load products - did you include products.js?");
        return;
    }

    configureSiteHead();
    configureSiteNavbar();
    configureSiteFooter();
});

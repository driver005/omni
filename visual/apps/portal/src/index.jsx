import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import App from './app';
import {probeCongeladoSdk} from './utils/congelado';

const ROOT_DIV_ID = 'ghost-portal-root';

function addRootDiv() {
  const elem = document.createElement('div');
  elem.id = ROOT_DIV_ID;
  elem.setAttribute('data-testid', 'portal-root');
  document.body.appendChild(elem);
}

function getSiteData() {
  /**
   * @type {HTMLElement}
   */
  const scriptTag = document.querySelector('script[data-ghost]');
  if (scriptTag) {
    const siteI18nEnabled = scriptTag.dataset.i18n === 'true';
    const siteUrl = scriptTag.dataset.ghost;
    const apiKey = scriptTag.dataset.key;
    const apiUrl = scriptTag.dataset.api;
    const locale = scriptTag.dataset.locale; // not providing a fallback here but will do it within the app.
    const useCongeladoSdk = scriptTag.dataset.congelado === 'true';
    return { siteUrl, apiKey, apiUrl, siteI18nEnabled, locale, useCongeladoSdk };
  }
  return {};
}

function handleTokenUrl() {
  const url = new URL(window.location.href);
  if (url.searchParams.get('token')) {
    url.searchParams.delete('token');
    window.history.replaceState({}, document.title, url.href);
  }
}

function setPreviewTheme() {
  const [, queryString] = window.location.hash.substring(1).split('?');
  const previewTheme = new URLSearchParams(queryString).get('previewTheme');

  if (previewTheme === 'dark') {
    document.documentElement.dataset.portalPreviewTheme = 'dark';
  }
}

function init() {
  // const customSiteUrl = getSiteUrl();
  const { siteUrl: customSiteUrl, apiKey, apiUrl, siteI18nEnabled, locale, useCongeladoSdk } = getSiteData();
  const siteUrl = customSiteUrl || window.location.origin;

  setPreviewTheme();
  addRootDiv();
  handleTokenUrl();

  if (useCongeladoSdk) {
    probeCongeladoSdk(siteUrl)
      .then(status => console.info('[congelado] posts request status', status))
      .catch(error => console.warn('[congelado] SDK request failed', error));
  }

  ReactDOM.render(
    <React.StrictMode>
      <App
        siteUrl={siteUrl}
        customSiteUrl={customSiteUrl}
        apiKey={apiKey}
        apiUrl={apiUrl}
        siteI18nEnabled={siteI18nEnabled}
        locale={locale}
      />
    </React.StrictMode>,
    document.getElementById(ROOT_DIV_ID),
  );
}

init();
